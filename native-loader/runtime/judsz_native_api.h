#pragma once

#include <cstdint>

#if defined(__cplusplus)
extern "C" {
#endif

// Stable status codes shared by the Android/JNI bridge and native runtime.
enum JudszStatus : int32_t {
    JUDSZ_OK = 0,
    JUDSZ_ERR_NOT_INITIALIZED = 1,
    JUDSZ_ERR_UNSUPPORTED_VERSION = 2,
    JUDSZ_ERR_UNSUPPORTED_FEATURE = 3,
    JUDSZ_ERR_INVALID_ARGUMENT = 4,
    JUDSZ_ERR_INTERNAL = 5
};

// Metadata is informational only. A matching version is not proof that
// game-function addresses or bindings have been verified.
struct JudszRuntimeInfo {
    uint32_t api_version;
    uint32_t game_version_code;
    uint32_t abi_code;
    bool bindings_verified;
};

// Initializes Judsz's standalone native runtime. This function does not
// attach to or modify another app's process.
JudszStatus judsz_runtime_initialize();
JudszRuntimeInfo judsz_runtime_info();

// Dispatches a named feature through the runtime's allow-list. Until a
// verified game integration exists, gameplay-changing commands must return
// JUDSZ_ERR_UNSUPPORTED_FEATURE.
JudszStatus judsz_dispatch_command(const char* command, const char* json_args);

#if defined(__cplusplus)
}
#endif
