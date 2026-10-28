package com.gdmloader.android

import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.webkit.JavascriptInterface
import android.webkit.WebChromeClient
import android.webkit.WebResourceRequest
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.webkit.WebViewAssetLoader

private const val GEOMETRY_DASH_PACKAGE = "com.robtopx.geometryjump"

class MainActivity : Activity() {
    private lateinit var webView: WebView
    private lateinit var assetLoader: WebViewAssetLoader

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
            override fun shouldInterceptRequest(
                view: WebView?,
                request: WebResourceRequest
            ) = assetLoader.shouldInterceptRequest(request.url)

            override fun shouldOverrideUrlLoading(
                view: WebView?,
                request: WebResourceRequest
            ): Boolean {
                // Keep navigation inside the bundled app. External links do not open in this WebView.
                return request.url.host != "appassets.androidplatform.net"
            }
        }
        webView.addJavascriptInterface(GameDetectorBridge(), "GDMAndroidBridge")
        setContentView(webView)
        webView.loadUrl("https://appassets.androidplatform.net/assets/www/index.html")
    }

    private fun isGeometryDashInstalled(): Boolean {
        return try {
            if (android.os.Build.VERSION.SDK_INT >= 33) {
                packageManager.getPackageInfo(
                    GEOMETRY_DASH_PACKAGE,
                    PackageManager.PackageInfoFlags.of(0)
                )
            } else {
                @Suppress("DEPRECATION")
                packageManager.getPackageInfo(GEOMETRY_DASH_PACKAGE, 0)
            }
            true
        } catch (_: PackageManager.NameNotFoundException) {
            false
        }
    }

    private fun geometryDashVersion(): String {
        return try {
            val info = if (android.os.Build.VERSION.SDK_INT >= 33) {
                packageManager.getPackageInfo(
                    GEOMETRY_DASH_PACKAGE,
                    PackageManager.PackageInfoFlags.of(0)
                )
            } else {
                @Suppress("DEPRECATION")
                packageManager.getPackageInfo(GEOMETRY_DASH_PACKAGE, 0)
            }
            info.versionName ?: "Unknown"
        } catch (_: PackageManager.NameNotFoundException) {
            ""
        }
    }

    private inner class GameDetectorBridge {
        @JavascriptInterface
        fun isGeometryDashInstalled(): Boolean = this@MainActivity.isGeometryDashInstalled()

        @JavascriptInterface
        fun getGeometryDashVersion(): String = geometryDashVersion()

        @JavascriptInterface
        fun launchGeometryDash(): Boolean {
            if (!isGeometryDashInstalled()) return false
            return try {
                val intent = packageManager.getLaunchIntentForPackage(GEOMETRY_DASH_PACKAGE)
                    ?: return false
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivity(intent)
                true
            } catch (_: ActivityNotFoundException) {
                false
            } catch (_: SecurityException) {
                false
            }
        }
    }

    override fun onBackPressed() {
        if (::webView.isInitialized && webView.canGoBack()) {
            webView.goBack()
        } else {
            super.onBackPressed()
        }
    }

    override fun onDestroy() {
        if (::webView.isInitialized) {
            webView.removeJavascriptInterface("GDMAndroidBridge")
            webView.destroy()
        }
        super.onDestroy()
    }
}
