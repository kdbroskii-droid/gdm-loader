# GDM Loader

Cross-platform Geometry Dash mod-manager companion and Judsz editor prototype. The responsive installable PWA runs on Android, Windows, and Linux browsers.

## MVP features
- Geode-inspired mod library with local install state and an online JSON catalog.
- Judsz editor with a restricted demo interpreter for `show_message`, `jump`, `rocketup`, and `set_player_speed`.
- Local profiles, optional local PIN field, and a Geometry Dash username note.
- Exportable/importable GDM Loader metadata recovery points.
- Safe mode, dependency prompts, offline app shell, and smoke-test CI.

## Honest limitations
This is not yet a native game-mod injector. Its independent UI cannot patch Geometry Dash or execute in-game hooks by itself. A version-specific native integration is still required. A browser/PWA cannot inspect arbitrary installed programs or access Geometry Dash's private Android save directory. The current recovery feature backs up GDM Loader metadata only, not Geometry Dash save files. The install path field is a note and does not scan or modify files.

The app does not request or store Geometry Dash passwords. Log in or register inside Geometry Dash itself. The local profile is not an online Judsz account; no account server or cloud sync is configured.

## Run
Open `index.html`, or serve the folder over HTTP:

```sh
python -m http.server 8080
```

Open `http://localhost:8080`. Service workers require HTTPS or localhost. Deploy to a HTTPS static host to install the PWA on Android.

## Judsz example
```judsz
mod "Starter Mod":
    on_level_start:
        show_message("Judsz is running!")
        jump()
        rocketup()
```

The demo interpreter simulates these commands in the console; it does not control the game.

## Roadmap
1. Validate the PWA and editor prototype.
2. Implement native Windows/Linux detection and real save-file backup.
3. Design a supported, version-specific native mod bridge.
4. Add signed packages, dependency resolution, rollback, and release builds.
5. Evaluate Android companion integration and its platform restrictions.

Catalog entries are data only. Downloaded code is never automatically executed.
