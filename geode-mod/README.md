# GD M — native Geode module (Android)

This folder is the native in-game integration track for GD M. It uses Geode's supported mod project structure and adds a GD M button to Geometry Dash's main menu. The button opens a status popup so the first native integration can be tested without pretending the rest is already complete.

## Build requirements

- A Geometry Dash Android installation compatible with the chosen Geode SDK build.
- Geode CLI and Android NDK configured on the development machine.
- Matching Android Geode SDK binaries.

From this directory, install the Android 64-bit SDK binaries and build:

```sh
geode sdk install-binaries -p android64
geode build -p android64
```

For a 32-bit Geometry Dash build, use `android32` instead. The output `.geode` package is normally written under `build-android64/` or `build-android32/`. Install it through Geode's supported workflow. On Android, the documented mods directory is under `Android/media/com.geode.launcher/game/geode/mods/`.

Check the installed game's version and the current Geode release before building or installing. `mod.json` currently declares Geometry Dash Android 2.2081 and Geode v5.11.2 as the target; change those values only after testing against another combination.

## Current scope

- Uses Geode's real native mod lifecycle.
- Adds an in-game menu button and a status popup.
- Does not yet install, update, enable, or remove other mods.
- Does not yet execute Judsz in-game or map `jump()` / `rocketup()` to game events.
- Does not access or overwrite Geometry Dash save files.
- Does not provide online account registration.

Geode's own mod browser and dependency handling remain the supported way to install native mods in this milestone. The separate root-level PWA is a companion prototype; it is not yet connected to this native module.
