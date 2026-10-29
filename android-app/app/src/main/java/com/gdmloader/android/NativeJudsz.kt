package com.gdmloader.android

/**
 * Native Judsz runtime embedded in the Judsz app.
 *
 * This loads and tests Judsz's own native library. It does not inject into,
 * attach to, or modify Geometry Dash. Game bindings remain disabled until
 * a specific integration is verified.
 */
object NativeJudsz {
    private var loadError: String? = null
    private var initialized = false

    init {
        try {
            System.loadLibrary("judsz_native")
            initialized = nativeInitialize() == 0
        } catch (error: UnsatisfiedLinkError) {
            loadError = error.message ?: "Native library could not be loaded"
        } catch (error: Exception) {
            loadError = error.message ?: "Native initialization failed"
        }
    }

    @JvmStatic external fun nativeInitialize(): Int
    @JvmStatic external fun nativeRuntimeInfo(): String
    @JvmStatic external fun nativeDispatch(command: String, jsonArgs: String): Int

    @JvmStatic
    fun statusJson(): String {
        if (loadError != null) {
            val safe = loadError!!.replace("\\", "\\\\").replace("\"", "\\\"")
            return "{\"loaded\":false,\"initialized\":false,\"error\":\"$safe\"}"
        }
        return try {
            val info = nativeRuntimeInfo()
            "{\"loaded\":true,\"initialized\":$initialized,\"runtime\":$info}"
        } catch (error: Exception) {
            "{\"loaded\":true,\"initialized\":false,\"error\":\"status query failed\"}"
        }
    }
}
