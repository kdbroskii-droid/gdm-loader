# GD M / Judsz

GD M is an Android launcher/mod project for Geometry Dash. The integrated launcher workflow now uses the official Geode Android Launcher source as its native runtime foundation, builds the GD M Geode mod, bundles that mod inside the launcher assets, and rebrands the launcher as GD M / Judsz.

## Integrated launcher workflow

The new workflow is `.github/workflows/gdm-launcher.yml`. It:
1. Checks out this repository and the pinned upstream Geode Android Launcher release `1.9.0`.
2. Builds the current GD M mod for Android 64-bit.
3. Copies the resulting `.geode` package into the launcher's internal `assets/mods` directory. The upstream launcher copies internal mods into its managed Geode mods folder at game startup.
4. Rebrands the launcher application ID and main app labels to GD M / Judsz.
5. Builds a debug APK and uploads it as the `gdm-android-launcher-debug` workflow artifact.

This workflow has been committed but **has not been run**. Therefore there is not yet a verified APK artifact or device-tested result. The source is pinned to upstream tag `1.9.0` for reproducibility; review upstream updates before changing that pin.

## Current limitations

- The old `android-app/` project remains a separate WebView companion and APK inspector. It is not the native game launcher.
- The existing `geode-mod/` source is still a prototype. Auto Play is heuristic and not validated across all levels/modes.
- Unlimited Resources and Complete Everything are not implemented. They must not be represented as working features until real handlers exist and are tested.
- The upstream launcher keeps its own setup and recovery behavior. First launch may require setup and confirmation; automatic launching must only be enabled after setup succeeds.
- A successful build alone will not prove runtime compatibility. A real Android device smoke test is still required.

## Build artifact

To create the APK later, open the repository's **Actions** tab, select **Prepare GD M Android launcher APK**, and run the workflow manually. The APK will be available under the `gdm-android-launcher-debug` artifact if the workflow succeeds. No build was run as part of the source changes in this commit.

## Safety, compatibility, and licensing

GD M must not bundle or redistribute proprietary Geometry Dash APKs, assets, or game data. Users must obtain the game through legitimate channels. Keep backups of game saves. Modding can affect compatibility with updates and online features. GD M is not affiliated with RobTop Games or the Geode team.

The launcher source is licensed under the Boost Software License 1.0. The upstream license text is retained in `licenses/GEODE-ANDROID-LAUNCHER-BOOST-1.0.txt`; any redistributed derivative must preserve required upstream notices. See the [official launcher repository](https://github.com/geode-sdk/android-launcher).
