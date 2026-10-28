# GDM Loader — Android + Geode

GDM Loader is an Android-first Geometry Dash modding project with two parts:

- **Native Geode module** in `geode-mod/`: a real Geode mod project that adds a GDM Loader button to Geometry Dash's main menu. This is the first native integration milestone.
- **Native Android app** in `android-app/`: checks whether Geometry Dash is installed, reports its version, and can launch it. It bundles the companion UI so it can call Android package APIs.
- **Companion PWA** at the repository root: responsive mod catalog prototype, local profile UI, metadata recovery, and restricted Judsz demo editor. The browser-only PWA cannot inspect installed Android packages.

## Native Android app build

The native companion app checks the `com.robtopx.geometryjump` package through Android `PackageManager`, shows whether the game is installed and its version, and offers an Open Geometry Dash button when detected. Android 11+ package visibility is limited to that package in the manifest; it does not request broad app-list access.

With Android SDK Platform 35, JDK 17, and Gradle 8.9 installed, run from the repository root:

```sh
gradle -p android-app assembleDebug
```

The debug APK is produced at `android-app/app/build/outputs/apk/debug/app-debug.apk`. The build and package check still need testing on a physical device.

On every push, GitHub Actions also runs the Android build and uploads an artifact named `gdm-loader-android-debug-apk`. Open the repository's [Actions page](https://github.com/kdbroskii-droid/gdm-loader/actions), select the latest `Validate GDM Loader Android project` run, and download that artifact if the Android build job succeeds.

## Native Geode mod build

The native module targets Geometry Dash Android 2.2081 and Geode v5.11.2 in `geode-mod/mod.json`. Verify the installed Geometry Dash version and the current Geode release before building; these compatibility values must be tested on the actual device.

Install the Geode CLI and Android NDK, then from `geode-mod/` run:

```sh
geode sdk install-binaries -p android64
geode build -p android64
```

For a 32-bit game build, use `android32`. The built `.geode` package is normally placed in `build-android64/` or `build-android32/`. Install it using Geode's supported Android workflow. The documented Android mods directory is `Android/media/com.geode.launcher/game/geode/mods/`.

Official documentation: [Geode SDK docs](https://docs.geode-sdk.org) and [Geode Android mod build guide](https://github.com/poweredbypie/geode.docs/blob/main/getting-started/create-mod.md).

## Current native capabilities

- Uses Geode's native mod lifecycle.
- Adds a main-menu button and opens a status popup.
- Leaves actual native mod installation, dependency resolution, and updates to Geode's own mod manager.
- Does **not** yet install/manage other mods from a custom GDM Loader browser.
- Does **not** yet execute Judsz scripts in-game or map `jump()` / `rocketup()` to game events.
- Does **not** access or overwrite Geometry Dash save files.
- Does **not** provide an online Judsz account backend.

The companion PWA is not yet connected to the native module. PWA recovery currently includes GDM Loader metadata only, not Geometry Dash's private save data. Its local profile is not an online account.

## Android companion PWA

Serve the repository root over HTTPS to install it from a supported Android browser. For local development:

```sh
python -m http.server 8080
```

Then open `http://localhost:8080`. Service workers require HTTPS or localhost.

## Judsz example

```judsz
mod "Starter Mod":
    on_level_start:
        show_message("Judsz is running!")
        jump()
        rocketup()
```

The current PWA demo interpreter simulates these commands; it does not control the game. The native Judsz bridge remains future work.

## Validation

Run the repository's JavaScript smoke tests with Node.js:

```sh
npm test
```

This validates file presence and metadata, not the native Android binary. A real native build still needs the Geode CLI, Android NDK, matching SDK binaries, and testing on a compatible Geometry Dash/Geode installation.

GDM Loader is an independent community project and is not affiliated with RobTop Games or the Geode team.
