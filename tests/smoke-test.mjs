import { readFileSync, existsSync } from "node:fs";
import assert from "node:assert/strict";

const required = [
  "index.html", "app.css", "app.js", "catalog.json",
  "android-app/settings.gradle.kts", "android-app/build.gradle.kts",
  "android-app/app/build.gradle.kts", "android-app/app/src/main/AndroidManifest.xml",
  "android-app/app/src/main/java/com/gdmloader/android/MainActivity.kt",
  "android-app/app/src/main/assets/www/index.html",
  "android-app/app/src/main/assets/www/app.js",
  "manifest.webmanifest", "sw.js", "assets/icon.svg",
  "geode-mod/CMakeLists.txt", "geode-mod/mod.json", "geode-mod/src/main.cpp",
  "native-loader/runtime/judsz_native_api.cpp",
  "native-loader/autoplay/autoplay_planner.h",
  "native-loader/autoplay/autoplay_planner.cpp",
  "native-loader/autoplay/README.md",
  "backend/supabase-schema.sql"
];
for (const file of required) assert.ok(existsSync(file), "Missing " + file);

const html = readFileSync("index.html", "utf8");
const js = readFileSync("app.js", "utf8");
const manifest = JSON.parse(readFileSync("manifest.webmanifest", "utf8"));
const catalog = JSON.parse(readFileSync("catalog.json", "utf8"));
const geodeManifest = JSON.parse(readFileSync("geode-mod/mod.json", "utf8"));
const nativeMod = readFileSync("geode-mod/src/main.cpp", "utf8");
const androidManifest = readFileSync("android-app/app/src/main/AndroidManifest.xml", "utf8");
const androidMain = readFileSync("android-app/app/src/main/java/com/gdmloader/android/MainActivity.kt", "utf8");
const androidHtml = readFileSync("android-app/app/src/main/assets/www/index.html", "utf8");
const androidJs = readFileSync("android-app/app/src/main/assets/www/app.js", "utf8");
const nativeRuntime = readFileSync("native-loader/runtime/judsz_native_api.cpp", "utf8");
const plannerHeader = readFileSync("native-loader/autoplay/autoplay_planner.h", "utf8");
const plannerSource = readFileSync("native-loader/autoplay/autoplay_planner.cpp", "utf8");
const cloudSql = readFileSync("backend/supabase-schema.sql", "utf8");

assert.match(html, /id="game"/);
assert.match(html, /id="dictionary"/);
assert.match(js, /function draw\(/);
assert.match(js, /GDMAndroidBridge/);
assert.match(androidJs, /GDMAndroidBridge/);
assert.match(androidHtml, /id="game-status"/);
assert.match(androidManifest, /com\.robtopx\.geometryjump/);
assert.match(androidMain, /launchGeometryDash/);
assert.equal(manifest.name, "GD M");
assert.ok(Array.isArray(catalog.mods));
assert.equal(geodeManifest.id, "kdbroskii.gdm-loader");
assert.deepEqual(geodeManifest.gd, { android: "2.2081" });
assert.match(nativeMod, /class \$modify\(GDMAutoPlayLayer, PlayLayer\)/);
assert.match(nativeMod, /getObjectRect/);
assert.match(nativeMod, /PlayerButton::Jump/);
assert.match(nativeMod, /currently loaded level/);
assert.match(plannerHeader, /struct Hitbox/);
assert.match(plannerHeader, /struct Obstacle/);
assert.match(plannerSource, /predict_obstacle_box/);
assert.match(plannerSource, /Action::Jump/);
assert.match(nativeRuntime, /JUDSZ_ERR_UNSUPPORTED_FEATURE/);
assert.match(cloudSql, /enable row level security/i);

console.log("Judsz app, Android bridge, Geode hitbox autoplay, and native planner smoke tests passed.");
