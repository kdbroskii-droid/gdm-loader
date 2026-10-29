package com.gdmloader.android

import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Environment
import android.provider.Settings
import java.io.File
import android.os.Bundle
import android.webkit.JavascriptInterface
import android.webkit.WebChromeClient
import android.webkit.WebResourceRequest
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.documentfile.provider.DocumentFile
import androidx.webkit.WebViewAssetLoader
import org.json.JSONArray
import org.json.JSONObject

private const val GEOMETRY_DASH_PACKAGE = "com.robtopx.geometryjump"
private const val PREFS = "gdm_native"
private const val TREE_URI = "workspace_tree_uri"
private const val PICK_TREE = 4107

class MainActivity : Activity() {
    private lateinit var webView: WebView
    private lateinit var assetLoader: WebViewAssetLoader
    private val prefs by lazy { getSharedPreferences(PREFS, MODE_PRIVATE) }
    private var pickerPending = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        assetLoader = WebViewAssetLoader.Builder()
            .addPathHandler("/assets/", WebViewAssetLoader.AssetsPathHandler(this))
            .build()
        webView = WebView(this)
        webView.settings.javaScriptEnabled = true
        webView.settings.domStorageEnabled = true
        webView.settings.allowFileAccess = false
        webView.settings.allowContentAccess = false
        webView.settings.javaScriptCanOpenWindowsAutomatically = false
        webView.settings.setSupportMultipleWindows(false)
        webView.webChromeClient = WebChromeClient()
        webView.webViewClient = object : WebViewClient() {
            override fun shouldInterceptRequest(view: WebView?, request: WebResourceRequest) =
                assetLoader.shouldInterceptRequest(request.url)
            override fun shouldOverrideUrlLoading(view: WebView?, request: WebResourceRequest): Boolean =
                request.url.host != "appassets.androidplatform.net"
        }
        webView.addJavascriptInterface(GameDetectorBridge(), "GDMAndroidBridge")
        setContentView(webView)
        webView.loadUrl("https://appassets.androidplatform.net/assets/www/index.html")
    }

    private fun savedTree(): DocumentFile? {
        val raw = prefs.getString(TREE_URI, null) ?: return null
        return try {
            val tree = DocumentFile.fromTreeUri(this, Uri.parse(raw))
            if (tree != null && tree.exists() && tree.canRead()) tree else null
        } catch (_: Exception) { null }
    }

    private fun chooseWorkspaceFolder() {
        if (pickerPending || isFinishing) return
        pickerPending = true
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT_TREE).apply {
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION or Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION)
            putExtra("android.content.extra.SHOW_ADVANCED", true)
        }
        try { startActivityForResult(intent, PICK_TREE) }
        catch (_: ActivityNotFoundException) {
            pickerPending = false
            notifyWeb("workspacePickerResult", false, "No Android folder picker is available.")
        }
    }

    @Deprecated("Activity result API retained for compatibility with minSdk 23")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != PICK_TREE) return
        pickerPending = false
        if (resultCode != RESULT_OK || data?.data == null) {
            notifyWeb("workspacePickerResult", false, "Folder access was not granted. You can choose a folder in Settings.")
            return
        }
        val uri = data.data!!
        val flags = data.flags and (Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
        try {
            contentResolver.takePersistableUriPermission(uri, flags)
            prefs.edit().putString(TREE_URI, uri.toString()).apply()
            notifyWeb("workspacePickerResult", true, "Folder connected. GD M can read and write files inside the folder you selected.")
        } catch (e: Exception) {
            notifyWeb("workspacePickerResult", false, "Could not keep folder access: " + (e.message ?: "unknown error"))
        }
    }

    private fun notifyWeb(function: String, success: Boolean, message: String) {
        if (!::webView.isInitialized) return
        val payload = JSONObject().put("success", success).put("message", message)
        runOnUiThread { webView.evaluateJavascript("window.$function && window.$function(${payload})", null) }
    }

    private fun hasAllFilesAccess(): Boolean = android.os.Build.VERSION.SDK_INT < 30 || Environment.isExternalStorageManager()

    private fun requestAllFilesAccess() {
        if (android.os.Build.VERSION.SDK_INT < 30 || Environment.isExternalStorageManager()) return
        try { startActivity(Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, Uri.parse("package:$packageName"))) }
        catch (_: Exception) { try { startActivity(Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)) } catch (_: Exception) {} }
    }

    private fun installedApps(): String {
        val out = JSONArray()
        val pm = packageManager
        val apps = if (android.os.Build.VERSION.SDK_INT >= 33) pm.getInstalledApplications(PackageManager.ApplicationInfoFlags.of(0)) else {
            @Suppress("DEPRECATION")
            pm.getInstalledApplications(0)
        }
        apps.sortedBy { pm.getApplicationLabel(it).toString().lowercase() }.forEach { app ->
            out.put(JSONObject().put("name", pm.getApplicationLabel(app).toString()).put("packageName", app.packageName)
                .put("systemApp", (app.flags and android.content.pm.ApplicationInfo.FLAG_SYSTEM) != 0)
                .put("launchable", pm.getLaunchIntentForPackage(app.packageName) != null))
        }
        return out.toString()
    }

    private fun deviceFiles(): String {
        if (!hasAllFilesAccess()) return JSONObject().put("error", "All files access is not granted.").toString()
        val out = JSONArray()
        val root = Environment.getExternalStorageDirectory()
        fun walk(dir: File, depth: Int) {
            if (depth > 4 || out.length() >= 1200) return
            val children = try { dir.listFiles()?.sortedBy { it.name.lowercase() } ?: return } catch (_: SecurityException) { return }
            for (child in children) {
                if (child.name == "Android") { out.put(JSONObject().put("path", "Android").put("directory", true).put("restricted", true).put("size", 0)); continue }
                val rel = child.absolutePath.removePrefix(root.absolutePath).trimStart('/')
                out.put(JSONObject().put("path", rel).put("directory", child.isDirectory).put("size", if (child.isFile) child.length() else 0).put("modified", child.lastModified()))
                if (child.isDirectory) walk(child, depth + 1)
            }
        }
        walk(root, 0)
        return out.toString()
    }

    private fun readDeviceFile(path: String): String {
        if (!hasAllFilesAccess()) return JSONObject().put("error", "All files access is not granted").toString()
        val root = Environment.getExternalStorageDirectory().canonicalFile
        return try {
            val file = File(root, path).canonicalFile
            if (!file.path.startsWith(root.path + File.separator) || !file.isFile) return JSONObject().put("error", "File not found or path rejected").toString()
            if (file.length() > 1000000) return JSONObject().put("error", "File exceeds 1 MB limit").toString()
            JSONObject().put("path", path).put("content", file.readText(Charsets.UTF_8)).toString()
        } catch (e: Exception) { JSONObject().put("error", e.message ?: "Read failed").toString() }
    }

    private fun writeDeviceFile(path: String, content: String): String {
        if (!hasAllFilesAccess()) return JSONObject().put("success", false).put("message", "All files access is not granted").toString()
        if (content.length > 1000000) return JSONObject().put("success", false).put("message", "File exceeds 1 MB limit").toString()
        val root = Environment.getExternalStorageDirectory().canonicalFile
        return try {
            val file = File(root, path).canonicalFile
            if (!file.path.startsWith(root.path + File.separator) || file.isDirectory) return JSONObject().put("success", false).put("message", "Path rejected").toString()
            file.parentFile?.mkdirs()
            file.writeText(content, Charsets.UTF_8)
            JSONObject().put("success", true).put("message", "Saved $path").toString()
        } catch (e: Exception) { JSONObject().put("success", false).put("message", e.message ?: "Write failed").toString() }
    }

    private fun packageInfo() = if (android.os.Build.VERSION.SDK_INT >= 33) {
        packageManager.getPackageInfo(GEOMETRY_DASH_PACKAGE, PackageManager.PackageInfoFlags.of(0))
    } else {
        @Suppress("DEPRECATION")
        packageManager.getPackageInfo(GEOMETRY_DASH_PACKAGE, 0)
    }
    private fun isGeometryDashInstalled(): Boolean = try { packageInfo(); true }
        catch (_: PackageManager.NameNotFoundException) { false }
    private fun geometryDashVersion(): String = try { packageInfo().versionName ?: "Unknown" }
        catch (_: PackageManager.NameNotFoundException) { "" }

    private fun safeRelativePath(path: String): List<String>? {
        if (path.isBlank() || path.startsWith("/") || path.contains("\\") || path.split("/").any { it.isBlank() || it == "." || it == ".." }) return null
        if (path.length > 240 || path.split("/").any { it.length > 100 }) return null
        return path.split("/")
    }

    private fun resolveFile(path: String, create: Boolean): DocumentFile? {
        val parts = safeRelativePath(path) ?: return null
        var dir = savedTree() ?: return null
        for (part in parts.dropLast(1)) {
            dir = dir.findFile(part)?.takeIf { it.isDirectory }
                ?: if (create) dir.createDirectory(part) ?: return null else return null
        }
        val name = parts.last()
        return dir.findFile(name)?.takeIf { it.isFile } ?: if (create) dir.createFile(mimeType(name), name) else null
    }

    private fun mimeType(name: String): String = when (name.substringAfterLast('.', "").lowercase()) {
        "json" -> "application/json"
        "txt", "judsz", "md", "log" -> "text/plain"
        "zip" -> "application/zip"
        "png" -> "image/png"
        "jpg", "jpeg" -> "image/jpeg"
        else -> "application/octet-stream"
    }

    private fun listFiles(): String {
        val root = savedTree() ?: return JSONArray().toString()
        val out = JSONArray()
        fun walk(dir: DocumentFile, prefix: String, depth: Int) {
            if (depth > 5 || out.length() >= 500) return
            dir.listFiles().sortedBy { it.getName()?.lowercase() ?: "" }.forEach { item ->
                val name = item.getName() ?: return@forEach
                val rel = if (prefix.isEmpty()) name else "$prefix/$name"
                out.put(JSONObject().put("path", rel).put("directory", item.isDirectory).put("size", item.length()).put("modified", item.lastModified()))
                if (item.isDirectory) walk(item, rel, depth + 1)
            }
        }
        walk(root, "", 0)
        return out.toString()
    }

    private fun readFile(path: String): String {
        val file = resolveFile(path, false) ?: return JSONObject().put("error", "File not found or path rejected").toString()
        if (file.length() > 1_000_000) return JSONObject().put("error", "File exceeds 1 MB read limit").toString()
        return try {
            val input = contentResolver.openInputStream(file.uri) ?: return JSONObject().put("error", "Could not open file").toString()
            val text = input.bufferedReader(Charsets.UTF_8).use { it.readText() }
            JSONObject().put("path", path).put("content", text).toString()
        } catch (e: Exception) { JSONObject().put("error", e.message ?: "Read failed").toString() }
    }

    private fun writeFile(path: String, content: String): String {
        if (content.length > 1_000_000) return JSONObject().put("success", false).put("message", "File exceeds 1 MB write limit").toString()
        val file = resolveFile(path, true) ?: return JSONObject().put("success", false).put("message", "Could not create file or path rejected").toString()
        return try {
            contentResolver.openOutputStream(file.uri, "wt")?.bufferedWriter(Charsets.UTF_8)?.use { it.write(content) }
                ?: return JSONObject().put("success", false).put("message", "Could not open file for writing").toString()
            JSONObject().put("success", true).put("message", "Saved $path").toString()
        } catch (e: Exception) { JSONObject().put("success", false).put("message", e.message ?: "Write failed").toString() }
    }

    private inner class GameDetectorBridge {
        @JavascriptInterface fun isGeometryDashInstalled(): Boolean = this@MainActivity.isGeometryDashInstalled()
        @JavascriptInterface fun getGeometryDashVersion(): String = geometryDashVersion()
        @JavascriptInterface fun launchGeometryDash(): Boolean {
            if (!isGeometryDashInstalled()) return false
            return try {
                val intent = packageManager.getLaunchIntentForPackage(GEOMETRY_DASH_PACKAGE) ?: return false
                startActivity(intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)); true
            } catch (_: ActivityNotFoundException) { false } catch (_: SecurityException) { false }
        }
        @JavascriptInterface fun getInstalledApps(): String = installedApps()
        @JavascriptInterface fun hasAllFilesAccess(): Boolean = this@MainActivity.hasAllFilesAccess()
        @JavascriptInterface fun requestAllFilesAccess() { runOnUiThread { this@MainActivity.requestAllFilesAccess() } }
        @JavascriptInterface fun listDeviceFiles(): String = deviceFiles()
        @JavascriptInterface fun readDeviceFile(path: String): String = readDeviceFile(path)
        @JavascriptInterface fun writeDeviceFile(path: String, content: String): String = writeDeviceFile(path, content)
        @JavascriptInterface fun hasWorkspaceAccess(): Boolean = savedTree() != null
        @JavascriptInterface fun getWorkspaceName(): String = savedTree()?.getName() ?: ""
        @JavascriptInterface fun chooseWorkspaceFolder() { runOnUiThread { this@MainActivity.chooseWorkspaceFolder() } }
        @JavascriptInterface fun listWorkspaceFiles(): String = listFiles()
        @JavascriptInterface fun readWorkspaceFile(path: String): String = readFile(path)
        @JavascriptInterface fun writeWorkspaceFile(path: String, content: String): String = writeFile(path, content)
    }

    override fun onBackPressed() {
        if (::webView.isInitialized && webView.canGoBack()) webView.goBack() else super.onBackPressed()
    }
    override fun onDestroy() {
        if (::webView.isInitialized) { webView.removeJavascriptInterface("GDMAndroidBridge"); webView.destroy() }
        super.onDestroy()
    }
}
