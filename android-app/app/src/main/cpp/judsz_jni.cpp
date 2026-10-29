#include <jni.h>
#include <string>
#include "judsz_native_api.h"

namespace {
jstring toJavaString(JNIEnv* env, const std::string& value) {
    return env->NewStringUTF(value.c_str());
}
}

extern "C" JNIEXPORT jint JNICALL
Java_com_gdmloader_android_NativeJudsz_nativeInitialize(JNIEnv*, jobject) {
    return static_cast<jint>(judsz_runtime_initialize());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_gdmloader_android_NativeJudsz_nativeRuntimeInfo(JNIEnv* env, jobject) {
    const JudszRuntimeInfo info = judsz_runtime_info();
    const std::string json =
        std::string("{\"apiVersion\":") + std::to_string(info.api_version) +
        ",\"gameVersionCode\":" + std::to_string(info.game_version_code) +
        ",\"abiCode\":" + std::to_string(info.abi_code) +
        ",\"bindingsVerified\":" + (info.bindings_verified ? "true" : "false") +
        ",\"integrationStatus\":\"scaffold_only\"}";
    return toJavaString(env, json);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_gdmloader_android_NativeJudsz_nativeDispatch(JNIEnv* env, jobject,
                                                      jstring command, jstring args) {
    if (command == nullptr) return static_cast<jint>(JUDSZ_ERR_INVALID_ARGUMENT);
    const char* commandChars = env->GetStringUTFChars(command, nullptr);
    if (commandChars == nullptr) return static_cast<jint>(JUDSZ_ERR_INTERNAL);
    const char* argsChars = args ? env->GetStringUTFChars(args, nullptr) : nullptr;
    const JudszStatus status = judsz_dispatch_command(commandChars, argsChars);
    if (argsChars != nullptr) env->ReleaseStringUTFChars(args, argsChars);
    env->ReleaseStringUTFChars(command, commandChars);
    return static_cast<jint>(status);
}
