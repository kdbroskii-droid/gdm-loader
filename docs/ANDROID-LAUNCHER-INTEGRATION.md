# Android launcher integration notes

## Goal

GD M is an Android app which launches the compatible Geometry Dash runtime with a GD M Geode mod loaded. The launcher must not simply open the user's normal game package and must not claim to inject into another app's private process.

## Upstream foundation

Use the official [Geode Android Launcher](https://github.com/geode-sdk/android-launcher) as the source of truth for Android native launch, game activity setup, Geode installation/update, save redirection, ABI handling, and recovery behavior. The upstream project is licensed under Boost Software License 1.0. Preserve the full license and upstream copyright notices in any fork or derivative work. Track an explicit upstream tag/commit; do not silently follow an unpinned `main` branch in a reproducible release.

## Workflow now committed\n\nThe workflow at `.github/workflows/gdm-launcher.yml` checks out the pinned upstream launcher tag `1.9.0`, builds the GD M mod for Android64, places the resulting `.geode` package in the upstream launcher's internal `assets/mods` directory, changes the app ID/labels, and builds an installable debug APK artifact. This workflow has **not** been run, and the APK has not been built or tested. The old WebView app is still separate. The custom mod features below remain incomplete.\n\n## Remaining integration tasks

- [ ] Fork/port the upstream launcher source and Gradle wrapper into this repository.
- [ ] Rebrand launcher title, icons, theme, application ID, deep-link scheme, and user-facing strings to GD M / Judsz.
- [ ] Preserve upstream game activity and native library loading logic. Do not replace it with the current WebView companion activity.
- [ ] Include the GD M mod source as a launcher-managed component and install the built `.geode` package into the launcher's own game/mod directory before first game launch and after mod updates.
- [ ] Add version/checksum metadata for the bundled mod and avoid repeatedly reinstalling an unchanged mod.
- [ ] Ensure first launch guides the user through the upstream install/setup flow; subsequent launch may automatically start the game only after setup succeeds.
- [ ] Keep a visible way to open launcher settings or safe mode after a crash.
- [ ] Keep Geometry Dash binaries/assets user-provided through the official installation; do not download or redistribute proprietary game files.
- [ ] Replace placeholder controls in `geode-mod/src/main.cpp`: Auto Play, Unlimited Resources, and Complete Everything must have real handlers or be labelled unavailable/removed. Avoid claiming universal level support for a heuristic autoplay implementation.
- [ ] Confirm menu behavior in the main menu and gameplay, test landscape layouts and different aspect ratios, test 32-bit/64-bit support as applicable, and verify game saves remain intact.

## Acceptance criteria before calling it complete

1. A clean install of GD M opens its setup screen without crashing.
2. With a compatible, legitimately installed Geometry Dash copy and supported Geode runtime, the launcher starts the modified game.
3. The game's main menu displays the floating cyan M button.
4. Tapping M opens/closes the custom blue/cyan mod panel without blocking normal game controls.
5. Every visible toggle either performs a tested in-game action or is visibly disabled with a clear explanation.
6. Force-close/relaunch and safe-mode recovery work.
7. CI builds the actual launcher APK (not only the old WebView companion) and the Geode mod package; a real-device smoke test is recorded separately.

## Current known gap

The current `android-app/` directory is only a WebView companion/inspector. The current `geode-mod/` is a prototype. The integration tasks above have not been completed merely because those projects compile. No build or device test was run while writing these notes.
