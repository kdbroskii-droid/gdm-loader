# Judsz Native Loader (Android)

This directory is the native integration layer for Judsz. It is intentionally separate from APK extraction: unpacking an APK does not create editable source code or hook a running process.

## Planned architecture

- `bridge/`: narrow JNI interface between the Android app and native code.
- `runtime/`: lifecycle, mod registry, logging, and safe command dispatch.
- `bindings/android/`: version-specific game bindings. Bindings must be verified against a particular Geometry Dash build before they are enabled.
- `mods/`: native or script-driven mods that use the public Judsz API.
- `tests/`: tests for dispatch and version matching.

## Current status

This folder documents and establishes the architecture only. It does **not** yet inject into Geometry Dash, hook game functions, or change live gameplay. The app's WebView/JavaScript bridge cannot directly modify another Android app's process.

## Integration route

1. Inspect the locally owned APK and record package/version/ABI metadata; do not commit the APK or its proprietary assets.
2. Build a native library with the Android NDK.
3. Integrate the library through an authorized, reproducible launch/package path for the selected game version.
4. Reverse-engineer and verify only the specific native interfaces required by a feature.
5. Enable bindings only when the detected game version and ABI match a tested profile.
6. Expose narrow commands to Judsz scripts; validate arguments and keep unsupported commands disabled.

For a first end-to-end test, use a small native test app owned by this project. Do not report a game hook as working until it has been verified on-device.
