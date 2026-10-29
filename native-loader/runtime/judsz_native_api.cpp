#include "judsz_native_api.h"

#include <cstring>

namespace {
bool g_initialized = false;
constexpr uint32_t kApiVersion = 1;
}

JudszStatus judsz_runtime_initialize() {
    g_initialized = true;
    return JUDSZ_OK;
}

JudszRuntimeInfo judsz_runtime_info() {
    JudszRuntimeInfo info{};
    info.api_version = kApiVersion;
    info.game_version_code = 0; // Filled only by a verified integration.
    info.abi_code = 0;          // Filled by the Android build/runtime.
    info.bindings_verified = false;
    return info;
}

JudszStatus judsz_dispatch_command(const char* command, const char* json_args) {
    if (!g_initialized) return JUDSZ_ERR_NOT_INITIALIZED;
    if (command == nullptr || command[0] == '\0') return JUDSZ_ERR_INVALID_ARGUMENT;
    (void)json_args;

    // Fail closed: no guessed addresses or unverified game calls.
    // Add a command here only when its backing binding has been verified.
    return JUDSZ_ERR_UNSUPPORTED_FEATURE;
}
