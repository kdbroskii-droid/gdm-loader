# Native integration milestones

- [x] Establish a native API boundary and explicit fail-closed status codes.
- [x] Document the difference between extraction and native runtime integration.
- [ ] Add Android NDK/CMake build configuration and JNI bridge.
- [ ] Build and test the native runtime inside a Judsz-owned Android test app.
- [ ] Add APK metadata inspection (version, ABI, native-library inventory) without publishing APK contents.
- [ ] Identify a lawful, reproducible integration route for the target Geometry Dash Android build.
- [ ] Implement and device-test one narrowly scoped native binding.
- [ ] Connect the Judsz script API to verified commands with argument validation.

A milestone is complete only when the build/test evidence exists. The current code is a scaffold, not a functioning Geometry Dash mod loader.
