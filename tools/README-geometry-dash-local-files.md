# Local Geometry Dash files for Judsz

The APK contains proprietary game assets and compiled native libraries. This repository does **not** store or redistribute those files. Extract your own APK locally into the ignored `local-game/geometry-dash-2.2081/` folder.

## Extract once

From the repository root, run:

```bash
python3 tools/extract_geometry_dash.py "/path/to/Geometry Dash_2.2081.apk"
```

On Windows, use `python` instead of `python3` if that is how Python is installed.

The script extracts the APK safely and writes a local inventory to `_judsz_extraction_manifest.json`. If the destination already exists, it leaves it alone, so you do not need to extract the APK again for each Judsz session. To deliberately replace it, pass `--force`.

## Important limitations

- This gives Judsz a persistent local folder of the APK's contents; it does not make the game run inside Judsz.
- APK resources such as PNG, plist, audio and data files are extracted as files. Native libraries such as `libcocos2dcpp.so` remain compiled binaries, not original source code.
- The extracted directory is in `.gitignore` and should not be committed to the public repository.
- Accessing or modifying the running game's memory/code still requires a separate, compatible native integration. This extraction script does not provide hooks or change the game.
