#!/usr/bin/env python3
"""Read-only inspection of a user-supplied Geometry Dash APK.

This tool never patches, signs, installs, or modifies the input APK.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

GAME_PACKAGE = "com.robtopx.geometryjump"
EXPECTED_SYMBOLS = (
    "PlayerObject::buttonDown",
    "PlayerObject::updateCollide(",
    "PlayerObject::updateCollideBottom",
    "PlayerObject::getCurrentXVelocity",
    "GJBaseGameLayer::playerCircleCollision",
    "GJBaseGameLayer::getGroundHeight",
    "GameObject::getBoundingRect",
    "GameObject::getObjectRect(",
    "PlayLayer::destroyPlayer",
    "PlayLayer::resetLevel",
)
MAX_APK_BYTES = 1_500_000_000


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def inspect_archive(apk: Path, abi: str) -> dict:
    """Return archive metadata without extracting persistent files."""
    report = {
        "file": apk.name,
        "sizeBytes": apk.stat().st_size,
        "sha256": sha256_file(apk),
        "packageExpected": GAME_PACKAGE,
        "abiRequested": abi,
        "manifestPresent": False,
        "zipIntegrity": "not_checked",
        "nativeLibraries": [],
        "symbolMatches": {},
        "metadataTool": None,
        "scope": "read-only inspection; no patching or injection performed",
    }
    with zipfile.ZipFile(apk) as archive:
        bad_entry = archive.testzip()
        report["zipIntegrity"] = "ok" if bad_entry is None else f"bad_entry:{bad_entry}"
        names = set(archive.namelist())
        report["manifestPresent"] = "AndroidManifest.xml" in names
        wanted = (
            f"lib/{abi}/libcocos2dcpp.so",
            f"lib/{abi}/libfmod.so",
        )
        report["nativeLibraries"] = [
            {"path": name, "sizeBytes": archive.getinfo(name).file_size}
            for name in wanted if name in names
        ]
        report["missingExpectedLibraries"] = [name for name in wanted if name not in names]
        readelf = shutil.which("readelf") or shutil.which("llvm-readelf")
        if readelf and any(item["path"].endswith("/libcocos2dcpp.so")
                           for item in report["nativeLibraries"]):
            with tempfile.TemporaryDirectory(prefix="gdm-apk-inspect-") as temp:
                library = Path(temp) / "libcocos2dcpp.so"
                library.write_bytes(archive.read(f"lib/{abi}/libcocos2dcpp.so"))
                report["nativeLibraries"][0 if report["nativeLibraries"][0]["path"].endswith("libcocos2dcpp.so") else 1]["sha256"] = sha256_file(library)
                proc = subprocess.run([readelf, "-Ws", str(library)], text=True,
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                      check=False)
                if proc.returncode == 0:
                    raw_names = []
                    for line in proc.stdout.splitlines():
                        fields = line.split()
                        if len(fields) >= 8 and fields[3] == "FUNC" and fields[6] != "UND":
                            raw_names.append(fields[7].split("@", 1)[0])
                    demangler = shutil.which("c++filt")
                    if demangler and raw_names:
                        demangled = subprocess.run(
                            [demangler], input="\n".join(raw_names) + "\n", text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
                        if demangled.returncode == 0:
                            raw_names = demangled.stdout.splitlines()
                    symbols = sorted(set(raw_names))
                    report["symbolMatches"] = {
                        needle: [symbol for symbol in symbols if needle in symbol][:8]
                        for needle in EXPECTED_SYMBOLS
                    }
                    report["symbolTool"] = Path(readelf).name
                else:
                    report["symbolInspectionError"] = proc.stderr.strip()
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path, help="path to an APK you are authorized to inspect")
    parser.add_argument("--abi", default="arm64-v8a", choices=("arm64-v8a", "armeabi-v7a"))
    parser.add_argument("--json", action="store_true", help="emit JSON")
    args = parser.parse_args()
    apk = args.apk.expanduser().resolve()
    if not apk.is_file():
        parser.error(f"APK not found: {apk}")
    if apk.stat().st_size > MAX_APK_BYTES:
        parser.error("APK exceeds 1.5 GB safety limit")
    try:
        report = inspect_archive(apk, args.abi)
    except (zipfile.BadZipFile, OSError, RuntimeError) as exc:
        print(json.dumps({"file": apk.name, "error": str(exc)}, indent=2)
              if args.json else f"Inspection failed: {exc}")
        return 2

    aapt = shutil.which("aapt") or shutil.which("aapt2")
    if aapt:
        proc = subprocess.run([aapt, "dump", "badging", str(apk)], text=True,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
        report["metadataTool"] = Path(aapt).name
        if proc.returncode == 0:
            report["badging"] = [line for line in proc.stdout.splitlines()
                                 if line.startswith(("package:", "sdkVersion:",
                                                     "targetSdkVersion:", "native-code:"))]
        else:
            report["metadataError"] = proc.stderr.strip()
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print(f"APK: {report['file']} ({report['sizeBytes']:,} bytes)")
        print(f"SHA-256: {report['sha256']}")
        print(f"ZIP integrity: {report['zipIntegrity']}")
        print(f"Manifest present: {report['manifestPresent']}")
        print(f"ABI: {args.abi}")
        for item in report["nativeLibraries"]:
            print(f"Native library: {item['path']} ({item['sizeBytes']:,} bytes)")
        if report.get("badging"):
            print("Android package metadata:\n" + "\n".join(report["badging"]))
        if report["symbolMatches"]:
            print("Native symbol candidates (not verified callable bindings):")
            for symbol, hits in report["symbolMatches"].items():
                print(f"  {symbol}: {len(hits)} match(es)")
                for hit in hits[:2]:
                    print(f"    {hit}")
        print("Scope: read-only inspection; this script does not patch the APK or load code into the game.")
    if report["zipIntegrity"] != "ok" or not report["manifestPresent"]:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
