# GD M / Judsz — standalone Geometry Dash mod loader

**Product target:** an Android app with its own mod menu and native runtime that can load mods into a compatible, user-patched Geometry Dash build. GD M is not intended to require the Geode launcher or a `.geode` plugin as its primary runtime.

## Important current status

- The Android companion APK builds successfully in GitHub Actions.
- The current app can detect and launch the installed Geometry Dash package.
- The native JNI library is still a scaffold. It does **not** inject into, patch, or run inside Geometry Dash.
- The existing Geode module is an optional prototype only; its successful build does **not** meet the standalone-loader requirement.
- The mod browser/editor UI and built-in test game are not yet connected to the real Geometry Dash process.
- No in-game mod-menu command or autoplay behavior has been verified on a device.

Do not interpret an Android APK build or a `.geode` build as proof of live game integration.

## Standalone design

See [`docs/STANDALONE-LOADER-ARCHITECTURE.md`](docs/STANDALONE-LOADER-ARCHITECTURE.md) for the target architecture and acceptance criteria.

A normal Android app cannot load native code into another installed app's private process simply by launching it. The intended route is a separate patched APK made from a user-selected local Geometry Dash APK. The patcher must preserve the source APK, verify supported package/version/ABI combinations, add the GD M native bridge, rebuild and sign the output, and reject unsupported inputs safely.

GD M must not download or redistribute proprietary Geometry Dash APKs/assets. Re-signing a modified APK may require uninstalling the original package first and can affect saves, purchases, updates, and online compatibility; back up important data before testing.

## Latest successful build

- **Status:** Successful — all CI jobs passed.
- **Built:** 29 October 2026.
- **Commit:** [`b89b590`](https://github.com/kdbroskii-droid/gdm-loader/commit/b89b590ab2381adb07e0e84874d81be88f49a44b).
- **Workflow run:** [View build and test results](https://github.com/kdbroskii-droid/gdm-loader/actions/runs/40913841575).
- **APK artifact:** [Download `gdm-loader-android-debug-apk`](https://github.com/kdbroskii-droid/gdm-loader/actions/runs/40913841575). Open the workflow run and use its **Artifacts** section. GitHub artifacts expire after a limited period.

This confirms the companion Android debug APK builds and the automated tests pass. It does **not** confirm live in-game injection or mod functionality; see [Important current status](#important-current-status).

## Existing Android companion build

Requirements: Android SDK Platform 35, JDK 17, and Gradle 8.9.

```sh
gradle -p android-app :app:assembleDebug
```

Output: `android-app/app/build/outputs/apk/debug/app-debug.apk`.

GitHub Actions builds the companion APK and uploads an artifact named `gdm-loader-android-debug-apk`.

## Native and game integration

The current native API reports `scaffold_only`. It must not report game bindings as verified until initialization inside the game process and real command dispatch have been tested. Mod-menu buttons must call the in-game runtime, not only JavaScript in the companion WebView.

Autoplay is a later gameplay feature: it must consume live collision/object data from the currently loaded level, avoid screenshots/textures/colours/OCR, and be tested across game modes. A rough collision planner alone is not a finished autoplay system.

## Validation

```sh
npm test
```

This runs repository smoke tests; it does not prove that the loader works inside Geometry Dash. Completion requires the acceptance tests in the architecture document, including a patched APK launching on a real device, a visible in-game GD M menu, and a test command producing a verified in-game change.

GD M is an independent community project and is not affiliated with RobTop Games or the Geode team.