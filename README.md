# GDM Loader — Android

Android-only companion app prototype for a Geometry Dash mod catalog and Judsz script editor.

## Android target
The current implementation is a responsive installable Progressive Web App (PWA), usable in an Android browser and installable from a HTTPS host. It is not yet a native APK/AAB. Windows/Linux desktop builds are out of scope.

## Included
- Online JSON catalog with local install-state labels ("Already installed").
- Judsz editor and restricted demo interpreter for `show_message`, `jump`, `rocketup`, and `set_player_speed`.
- Local profile and Geometry Dash username note.
- Exportable/importable GDM Loader metadata recovery points.
- Safe mode toggle, dependency prompts, offline shell and smoke-test CI.

## Important limitations
- This is not a native game-mod loader yet. The independent UI does not inject code into Geometry Dash; in-game hooks require a compatible, version-specific native integration.
- The catalog entries are illustrative metadata only. They do not modify the game.
- Browser/PWA storage cannot scan arbitrary installed programs or freely access Geometry Dash's private Android save directory. Recovery points currently contain GDM Loader metadata, not the game's save files.
- The local profile is not a remote Judsz account. There is no account backend or cloud sync.
- Do not enter your Geometry Dash password in this app. Sign in or register inside the game itself; this app has no official RobTop authentication integration.
- Catalog downloads are data only; downloaded code is never automatically executed.

## Run on Android
1. Host this repository on a static HTTPS host.
2. Open its hosted page in Chrome or another supported Android browser.
3. Use the browser menu's **Install app** or **Add to Home screen** action.

For local development on a computer, run `python -m http.server 8080` and open the page on that computer. This is only a development server; use HTTPS for normal Android PWA installation.

## Judsz example
```judsz
mod "Starter Mod":
    on_level_start:
        show_message("Judsz is running!")
        jump()
        rocketup()
```
These actions are simulated by the editor. They do not control Geometry Dash.

## Roadmap
1. Publish and test the Android PWA on real devices.
2. If a native APK is needed, package a mobile UI with an Android project and validate Android SDK/build requirements.
3. Investigate permitted, version-specific game integration and actual save backup restrictions.
4. Add real Judsz account registration only with a secure backend, email/account recovery, and clear privacy controls.
5. Add signed package manifests and release update verification.

GDM Loader is an independent community project and is not affiliated with RobTop Games or Geode.
