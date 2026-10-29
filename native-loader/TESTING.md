# Native runtime testing

## What can be tested now

The Android app now builds the native C++ runtime as `libjudsz_native.so` using CMake/NDK and loads it through JNI. The runtime exposes status reporting and an allow-listed command-dispatch boundary. Unsupported game commands intentionally return a status code instead of guessing game addresses.

## Build

Open `android-app` in Android Studio and install the Android SDK, NDK (side by side), and CMake from SDK Manager. Build the debug variant for a supported ABI.

The official NDK build model packages C/C++ code into the app and calls it through JNI; it does not grant access to another app's process.

## Validation checklist

- [ ] Gradle sync succeeds on a clean checkout.
- [ ] Debug APK builds for arm64-v8a.
- [ ] Judsz opens and `NativeJudsz.statusJson()` reports `loaded: true`.
- [ ] `nativeDispatch("test", "{}")` returns unsupported-feature status.
- [ ] No claim is made that Geometry Dash hooks are active until separately implemented and tested.

## Not implemented

No Geometry Dash process injection, no native game-function hooks, no address table, no decompiled proprietary game source, and no live gameplay modifications are included. These require a version-specific, tested integration path and cannot be inferred from the APK's extracted file tree.
