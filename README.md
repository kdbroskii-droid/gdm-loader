# GD M / Judsz

GD M is an independent Android launcher/mod project for Geometry Dash. The intended app should launch a locally installed, compatible Geometry Dash game through the Android native loader, then show GD M's own in-game menu (the floating cyan **M** button and blue/cyan panel).

## Current implementation status — do not mistake scaffolding for the app

The repository currently contains:
- A WebView-based companion UI and APK inspector in `android-app/`.
- A Geode mod prototype in `geode-mod/` with an M entry point and early Auto Play logic.
- Existing CI and native planner tests.

**The current Android app is not yet a Geode Android Launcher fork and does not launch a modded game runtime.** APK inspection is read-only; it does not patch the selected APK. The mod prototype's toggles for Unlimited Resources and Complete Everything are not implemented, and Auto Play has not been validated in-game. Do not label any of these features complete until they are implemented and tested.

## Correct implementation direction

Use the official [Geode Android Launcher source](https://github.com/geode-sdk/android-launcher) as the Android runtime/launch foundation rather than trying to inject into another app from a normal Android process. Its Android launcher handles native game startup, Geode setup, and game-specific compatibility work that this scaffold does not contain.

The intended integration work is:
1. Port/fork the official launcher source into this repository, retaining required upstream license notices.
2. Rebrand the app and package identifiers to GD M / Judsz without breaking the native launcher lifecycle, game activity, update path, or save redirection.
3. Package the GD M Geode mod with the launcher and install/update it into the launcher's own mod directory before starting the game.
4. Replace the mod's placeholder menu with the requested floating M button and responsive blue/cyan panel.
5. Implement each advertised toggle against real supported game APIs; do not show a toggle as functional until it has a working handler.
6. Make launch behavior recoverable: provide an explicit-launch/safe-mode route if automatic launch crashes, and never delete or overwrite the user's original game APK or saves.
7. Add automated source/manifest/package checks, then build and test on a real Android device separately.

See [the launcher integration notes](docs/ANDROID-LAUNCHER-INTEGRATION.md) for boundaries and acceptance criteria.

## Build status

No new build has been run for this change. A successful build of the existing companion APK or a `.geode` file would not prove that the integrated launcher works.

## Legal and safety boundaries

GD M must not bundle or redistribute proprietary Geometry Dash APKs, assets, or game data. Users must obtain the game through legitimate channels. A modified/re-signed game package can affect updates, save data, purchases, and online compatibility; keep backups and preserve the original installation. GD M is not affiliated with RobTop Games or the Geode team.

The official launcher is licensed under the Boost Software License 1.0; any port must retain the applicable notices and license text. See the [upstream repository](https://github.com/geode-sdk/android-launcher) for its current source and license.
