cmake_minimum_required(VERSION 3.22.1)
project(judsz_native LANGUAGES CXX)

add_library(judsz_native SHARED
    judsz_jni.cpp
    ../../../../../native-loader/runtime/judsz_native_api.cpp
)

target_include_directories(judsz_native PRIVATE
    ../../../../../native-loader/runtime
)

target_compile_features(judsz_native PRIVATE cxx_std_17)
target_compile_options(judsz_native PRIVATE -Wall -Wextra -Werror)
