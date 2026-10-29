# GD M / Judsz — standalone Geometry Dash loader

## Product requirement

GD M is intended to be a standalone Android loader with its own mod menu. It is **not** intended to require the Geode launcher or install a `.geode` plugin as its primary runtime.

The Android companion currently only detects and launches the installed game. The JNI library currently reports `scaffold_only`; it does not inject into or patch Geometry Dash. A successful APK build is not proof of in-game loading.

## Android integration boundary

An ordinary Android app cannot load a native library into another installed app's process just by calling its launch intent. Geometry Dash's private process and APK are isolated by Android. A real standalone implementation needs an explicit integration path, such as:

1. Build a user-authorized patched copy of a compatible Geometry Dash APK, add GD M's native runtime and startup bridge to that copy, then sign the modified APK with a development key; or
2. Use an officially supported mod-loader integration if the user deliberately chooses that route.

The primary goal is option 1 for a user-supplied APK. It must not silently modify the installed game, fetch copyrighted game assets, or redistribute RobTop's APK. Re-signing a patched copy may require uninstalling the original package first, and may affect cloud saves, purchases, updates, and multiplayer compatibility. Keep original APK and save backups.

## Required standalone architecture

- **Loader UI:** mod browser, enable/disable toggles, settings, script editor, logs, compatibility status, install/launch buttons.
- **APK intake:** Android Storage Access Framework; accept only a user-selected local APK; verify package ID, version, ABI, and archive integrity before changing anything.
- **Patcher/build pipeline:** use a pinned, auditable APK tooling pipeline to unpack the user-provided APK, add the GD M bridge and native libraries for the matching ABI, patch the startup path, rebuild, align, and sign. Fail closed on unsupported versions and preserve the original file.
- **Native runtime:** initialize inside the patched game's process, validate the game version and ABI, expose a narrow command API, and provide structured logs. Do not report `bindingsVerified=true` until in-process calls have been tested.
- **Mod menu bridge:** menu commands must reach the in-game runtime, not merely the companion WebView. Keep the UI-to-runtime protocol versioned and validate every command.
- **Game integration:** map each feature to verified game APIs and test on a real device. Hitbox autoplay must use live collision/object data and must not rely on screenshots, textures, colours, or OCR.
- **Build/test:** build the Android loader APK, build patched test APKs only from user-supplied game APKs, and run smoke tests for package detection, startup bridge, command dispatch, crash recovery, and version rejection.

## Current state (2026-10-29)

- Android companion APK builds in GitHub Actions.
- The Geode sample mod also builds, but that is a separate optional prototype and does not satisfy the standalone-loader requirement.
- The installed-game launch button works only as a launcher; it does not load mods into Geometry Dash.
- The native API reports `scaffold_only`; no verified in-process game bindings or patched APK pipeline are present yet.
- No actual gameplay test has established that a mod menu command changes the running game.

## Acceptance criteria

Do not mark the project complete until all of these pass:

1. A user selects a local compatible Geometry Dash APK.
2. GD M produces a separate patched APK and leaves the source APK unchanged.
3. Android installs and launches the patched build.
4. The GD M menu is visible inside the game.
5. A test mod command reaches the native runtime and causes a verified, observable in-game change.
6. Unsupported game versions/ABIs are rejected with a useful message.
7. A real-device test confirms no startup crash and records the game/app versions used.

The standalone path is the product. Geode code must not be presented as if it satisfies these criteria.

## User-supplied APK inspection (2026-10-29)

The user supplied a local file named `Geometry Dash_2.2081.apk` for compatibility investigation. Archive inspection found these native libraries:

- `lib/arm64-v8a/libcocos2dcpp.so`
- `lib/arm64-v8a/libfmod.so`
- `lib/armeabi-v7a/libcocos2dcpp.so`
- `lib/armeabi-v7a/libfmod.so`

This establishes that the APK archive contains ARM64 and ARMv7 native-library variants, including Cocos2d-x and FMOD. The filename alone is not authoritative proof of the package version; the binary Android manifest and package/version/signature details still need to be decoded and verified with Android tooling. No game files are committed to this repository. No patching or in-game integration has been performed from this inspection.
