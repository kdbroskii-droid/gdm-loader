import importlib.util
import tempfile
import unittest
import zipfile
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "inspect_gd_apk.py"
SPEC = importlib.util.spec_from_file_location("inspect_gd_apk", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ApkInspectionTests(unittest.TestCase):
    def make_apk(self, root: Path, entries: dict[str, bytes]) -> Path:
        path = root / "fixture.apk"
        with zipfile.ZipFile(path, "w") as archive:
            for name, content in entries.items():
                archive.writestr(name, content)
        return path

    def test_reports_manifest_and_native_abis(self):
        with tempfile.TemporaryDirectory() as tmp:
            apk = self.make_apk(Path(tmp), {
                "AndroidManifest.xml": b"manifest-fixture",
                "lib/arm64-v8a/libcocos2dcpp.so": b"native64",
                "lib/arm64-v8a/libfmod.so": b"fmod64",
                "lib/armeabi-v7a/libcocos2dcpp.so": b"native32",
                "lib/armeabi-v7a/libfmod.so": b"fmod32",
            })
            result = MODULE.inspect_archive(apk, "arm64-v8a")
            self.assertEqual(result["zipIntegrity"], "ok")
            self.assertTrue(result["manifestPresent"])
            self.assertEqual(len(result["nativeLibraries"]), 2)
            self.assertEqual(result["missingExpectedLibraries"], [])
            self.assertEqual(len(result["sha256"]), 64)

    def test_reports_missing_library(self):
        with tempfile.TemporaryDirectory() as tmp:
            apk = self.make_apk(Path(tmp), {"AndroidManifest.xml": b"manifest-fixture"})
            result = MODULE.inspect_archive(apk, "arm64-v8a")
            self.assertEqual(result["zipIntegrity"], "ok")
            self.assertEqual(len(result["missingExpectedLibraries"]), 2)

    def test_bad_zip_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "broken.apk"
            path.write_bytes(b"not a zip")
            with self.assertRaises(zipfile.BadZipFile):
                MODULE.inspect_archive(path, "arm64-v8a")


if __name__ == "__main__":
    unittest.main()
