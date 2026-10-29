#!/usr/bin/env python3
"""Inspect a user-supplied Geometry Dash APK without modifying it.

Requires Python 3.10+. For native symbol inspection, GNU/LLVM readelf must be
installed. This is an analysis helper, not an APK patcher or loader.
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


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run_readelf(readelf: str, library: Path) -> list[str]:
    proc = subprocess.run(
        [readelf, "-Ws", str(library)],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if proc.returncode:
        raise RuntimeError(f"readelf failed for {library.name}: {proc.stderr.strip()}")
    # Keep defined function names, then demangle C++ names for readable output.
    names: list[str] = []
    for line in proc.stdout.splitlines():
        fields = line.split()
        if len(fields) >= 8 and fields[3] == "FUNC" and fields[6] != "UND":
            names.append(fields[7].split("@", 1)[0])
    demangler = shutil.which("c++filt")
    if demangler and names:
        demangled = subprocess.run(
            [demangler], input="\n".join(names) + "\n", text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
        )
        if demangled.returncode == 0:
            return sorted(set(demangled.stdout.splitlines()))
    return sorted(set(names))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path, help="path to an APK you are authorized to inspect")
    parser.add_argument("--abi", default="arm64-v8a", choices=("arm64-v8a", "armeabi-v7a"))
    parser.add_argument("--json", action="store_true", help="emit JSON")
    args = parser.parse_args()
    apk = args.apk.expanduser().resolve()
    if not apk.is_file():
        parser.error(f"APK not found: {apk}")
    if apk.stat().st_size > 1_500_000_000:
        parser.error("APK exceeds 1.5 GB safety limit")

    report: dict = {
        "file": apk.name,
        "sizeBytes": apk.stat().st_size,
        "sha256": sha256_file(apk),
        "packageExpected": GAME_PACKAGE,
        "abiRequested": args.abi,
        "manifestPresent": False,
        "zipIntegrity": "not_checked",
        "nativeLibraries": [],
        "symbolMatches": {},
        "metadataTool": None,
        "scope": "read-only inspection; no patching or injection performed",
    }

    try:
        with zipfile.ZipFile(apk) as archive:
            bad_entry = archive.testzip()
            report["zipIntegrity"] = "ok" if bad_entry is None else f"bad_entry:{bad_entry}"
            report["manifestPresent"] = "AndroidManifest.xml" in archive.namelist()
            names = set(archive.namelist())
            wanted = (
                f"lib/{args.abi}/libcocos2dcpp.so",
                f"lib/{args.abi}/libfmod.so",
            )
            report["nativeLibraries"] = [
                {"path": name, "sizeBytes": archive.getinfo(name).file_size}
                for name in wanted if name in names
            ]
            missing = [name for name in wanted if name not in names]
            report["missingExpectedLibraries"] = missing
            with tempfile.TemporaryDirectory(prefix="gdm-apk-inspect-") as temp:
                tempdir = Path(temp)
                for item in report["nativeLibraries"]:
                    extracted = tempdir / Path(item["path"]).name
                    extracted.write_bytes(archive.read(item["path"]))
                    item["sha256"] = sha256_file(extracted)
                    if extracted.name == "libcocos2dcpp.so":
                        readelf = shutil.which("readelf") or shutil.which("llvm-readelf")
                        if readelf:
                            symbols = run_readelf(readelf, extracted)
                            matches = {}
                            for needle in EXPECTED_SYMBOLS:
                                hits = [symbol for symbol in symbols if needle in symbol]
                                matches[needle] = hits[:8]
                            report["symbolMatches"] = matches
                            report["symbolTool"] = Path(readelf).name
                        else:
                            report["symbolInspectionError"] = "Install readelf or llvm-readelf to inspect native symbols."

    except (zipfile.BadZipFile, OSError, RuntimeError) as exc:
        report["error"] = str(exc)
        print(json.dumps(report, indent=2) if args.json else f"Inspection failed: {exc}")
        return 2

    aapt = shutil.which("aapt") or shutil.which("aapt2")
    if aapt:
        proc = subprocess.run([aapt, "dump", "badging", str(apk)], text=True,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
        report["metadataTool"] = Path(aapt).name
        if proc.returncode == 0:
            report["badging"] = [line for line in proc.stdout.splitlines()
                                 if line.startswith(("package:", "sdkVersion:", "targetSdkVersion:", "native-code:"))]
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
            print("Android package metadata:")
            print("\n".join(report["badging"]))
        if report["symbolMatches"]:
            print("Native symbol candidates (names only; not verified callable bindings):")
            for symbol, hits in report["symbolMatches"].items():
                print(f"  {symbol}: {len(hits)} match(es)")
                for hit in hits[:2]:
                    print(f"    {hit}")
        elif report.get("symbolInspectionError"):
            print(report["symbolInspectionError"])
        print("Scope: read-only inspection; this script does not patch the APK or load code into the game.")
    if report["zipIntegrity"] != "ok" or not report["manifestPresent"]:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
