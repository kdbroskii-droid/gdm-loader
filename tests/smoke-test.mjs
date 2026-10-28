import {readFileSync, existsSync} from "node:fs";
import assert from "node:assert/strict";

for (const f of [
  "index.html", "app.css", "app.js", "catalog.json",
  "android-app/settings.gradle.kts", "android-app/build.gradle.kts",
  "android-app/app/build.gradle.kts", "android-app/app/src/main/AndroidManifest.xml",
  "android-app/app/src/main/java/com/gdmloader/android/MainActivity.kt",
  "android-app/app/src/main/assets/www/index.html", "android-app/app/src/main/assets/www/app.js", "android-app/app/src/main/assets/www/cloud-config.js",
  "manifest.webmanifest", "sw.js", "assets/icon.svg",
  "geode-mod/CMakeLists.txt", "geode-mod/mod.json",
  "geode-mod/src/main.cpp", "geode-mod/README.md", "cloud-config.js", "backend/supabase-schema.sql"
]) assert.ok(existsSync(f), "Missing " + f);

const html = readFileSync("index.html", "utf8");
const js = readFileSync("app.js", "utf8");
const cat = JSON.parse(readFileSync("catalog.json", "utf8"));
const manifest = JSON.parse(readFileSync("manifest.webmanifest", "utf8"));
const geode = JSON.parse(readFileSync("geode-mod/mod.json", "utf8"));
const native = readFileSync("geode-mod/src/main.cpp", "utf8");
const androidManifest = readFileSync("android-app/app/src/main/AndroidManifest.xml", "utf8");
const androidMain = readFileSync("android-app/app/src/main/java/com/gdmloader/android/MainActivity.kt", "utf8");
const androidHtml = readFileSync("android-app/app/src/main/assets/www/index.html", "utf8");
const androidJs = readFileSync("android-app/app/src/main/assets/www/app.js", "utf8");\nconst cloudSql = readFileSync("backend/supabase-schema.sql", "utf8");\nconst bundledCloudConfig = readFileSync("android-app/app/src/main/assets/www/cloud-config.js", "utf8");

assert.match(html, /id="page-judsz"/);
assert.match(html, /id="page-saves"/);
assert.match(js, /function run\(\)/);\nassert.match(js, /function lint\(/);\nassert.match(js, /cloudProfileSync/);\nassert.match(js, /15\*60\*1000/);\nassert.match(html, /id="cloud-auth-form"/);\nassert.match(html, /id="script-file"/);\nassert.match(readFileSync("backend/supabase-schema.sql","utf8"), /enable row level security/i);
assert.ok(cat.mods.length >= 3);
assert.ok(cat.mods.every(m => typeof m.id === "string" && Array.isArray(m.dependencies)));
assert.equal(manifest.name, "GDM Loader");
assert.equal(geode.id, "kdbroskii.gdm-loader");
assert.deepEqual(geode.gd, {android:"2.2081"});
assert.match(native, /class \$modify\(GDMMenuLayer, MenuLayer\)/);
assert.match(native, /menu_selector\(GDMMenuLayer::onOpenGDM\)/);
assert.match(androidManifest, /com\.robtopx\.geometryjump/);
assert.match(androidMain, /getPackageInfo/);
assert.match(androidMain, /launchGeometryDash/);
assert.match(js, /function refreshGameDetection\(\)/);
assert.match(androidHtml, /id="game-status"/);
assert.equal(androidJs, js, "Bundled Android UI must match root app.js");\nassert.match(cloudSql, /enable row level security/i);\nassert.match(cloudSql, /auth\.uid\(\) = user_id/);\nassert.match(bundledCloudConfig, /GDM_CLOUD_DEFAULTS/);
console.log("GDM Loader PWA and Android Geode-source smoke tests passed.");
