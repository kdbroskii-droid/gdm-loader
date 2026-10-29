#!/usr/bin/env python3
"""Extract a user-supplied Geometry Dash APK into an ignored local workspace.

This intentionally does not add the game's files to Git. Keep the APK you own
on your device/computer and run this once; the extracted directory persists
between Judsz launches.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path, PurePosixPath
import zipfile

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DEST = ROOT / "local-game" / "geometry-dash-2.2081"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path, help="path to your Geometry Dash APK")
    parser.add_argument("--dest", type=Path, default=DEFAULT_DEST,
                        help=f"extraction folder (default: {DEFAULT_DEST})")
    parser.add_argument("--force", action="store_true",
                        help="replace an existing extraction folder")
    args = parser.parse_args()

    apk = args.apk.expanduser().resolve()
    dest = args.dest.expanduser().resolve()

    if not apk.is_file():
        parser.error(f"APK file not found: {apk}")
    if not zipfile.is_zipfile(apk):
        parser.error("The input is not a valid ZIP/APK archive")
    if dest == ROOT or ROOT not in dest.parents:
        parser.error("Destination must be inside this repository")

    if dest.exists():
        if not args.force:
            print(f"Already extracted at: {dest}")
            print("Nothing changed. Use --force only if you want to replace it.")
            return 0
        import shutil
        shutil.rmtree(dest)

    dest.mkdir(parents=True, exist_ok=True)
    entries = []
    try:
        with zipfile.ZipFile(apk) as archive:
            for item in archive.infolist():
                name = item.filename
                rel = PurePosixPath(name)
                if rel.is_absolute() or ".." in rel.parts or not rel.parts:
                    raise ValueError(f"Unsafe archive path: {name!r}")
                target = dest.joinpath(*rel.parts).resolve()
                if dest != target and dest not in target.parents:
                    raise ValueError(f"Archive path escapes destination: {name!r}")
                if item.is_dir():
                    target.mkdir(parents=True, exist_ok=True)
                    continue
                target.parent.mkdir(parents=True, exist_ok=True)
                with archive.open(item) as source, target.open("wb") as output:
                    while True:
                        chunk = source.read(1024 * 1024)
                        if not chunk:
                            break
                        output.write(chunk)
                entries.append({"path": name, "size": item.file_size})
    except Exception:
        import shutil
        shutil.rmtree(dest, ignore_errors=True)
        raise

    metadata = {
        "source_apk_name": apk.name,
        "file_count": len(entries),
        "total_uncompressed_bytes": sum(entry["size"] for entry in entries),
        "files": entries,
        "note": "Local extracted copy only. Do not commit or redistribute proprietary game files."
    }
    (dest / "_judsz_extraction_manifest.json").write_text(
        json.dumps(metadata, indent=2), encoding="utf-8"
    )
    print(f"Extracted {len(entries)} files to: {dest}")
    print("This is an asset/binary extraction, not decompiled editable source code.")
    print("The extracted files are ignored by Git and will remain for future sessions.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
