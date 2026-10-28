import {readFileSync, existsSync} from "node:fs";
import assert from "node:assert/strict";

for (const f of [
  "index.html", "app.css", "app.js", "catalog.json",
  "manifest.webmanifest", "sw.js", "assets/icon.svg",
  "geode-mod/CMakeLists.txt", "geode-mod/mod.json",
  "geode-mod/src/main.cpp", "geode-mod/README.md"
]) assert.ok(existsSync(f), "Missing " + f);

const html = readFileSync("index.html", "utf8");
const js = readFileSync("app.js", "utf8");
const cat = JSON.parse(readFileSync("catalog.json", "utf8"));
const manifest = JSON.parse(readFileSync("manifest.webmanifest", "utf8"));
const geode = JSON.parse(readFileSync("geode-mod/mod.json", "utf8"));
const native = readFileSync("geode-mod/src/main.cpp", "utf8");

assert.match(html, /id="page-judsz"/);
assert.match(html, /id="page-saves"/);
assert.match(js, /function run\(\)/);
assert.ok(cat.mods.length >= 3);
assert.ok(cat.mods.every(m => typeof m.id === "string" && Array.isArray(m.dependencies)));
assert.equal(manifest.name, "GDM Loader");
assert.equal(geode.id, "kdbroskii.gdm-loader");
assert.deepEqual(geode.gd, {android:"2.2081"});
assert.match(native, /class \$modify\(GDMMenuLayer, MenuLayer\)/);
assert.match(native, /menu_selector\(GDMMenuLayer::onOpenGDM\)/);
console.log("GDM Loader PWA and Android Geode-source smoke tests passed.");
