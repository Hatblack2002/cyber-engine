# Cyber Engine — Android Gradle build
# Uses NDK r27 with arm64-v8a, GLES3, Vulkan NOT_IMPLEMENTED (per spec §5).
# Builds libcyberengine_jni.so via CMake, includes the engine + GLES renderer
# + JNI bridge + Linux PTY bridge (no Termux dependency).

cmake_minimum_required(VERSION 3.22.1)
project(cyberengine_jni LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Engine sources (host-agnostic) + GLES backend (Android only).
set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../engine)
set(JNI_ROOT    ${CMAKE_CURRENT_SOURCE_DIR}/../jni)

add_library(cyberengine_jni SHARED
    ${ENGINE_ROOT}/Engine.cpp
    ${ENGINE_ROOT}/shell/Shell.cpp
    ${ENGINE_ROOT}/scene/Scene.cpp
    ${ENGINE_ROOT}/renderer/gles/GLESRenderer.cpp
    ${JNI_ROOT}/cyber_jni.cpp
    ${JNI_ROOT}/linux_bridge.cpp
)

# Headers are referenced as "engine/Engine.hpp" — include path must be the
# parent of engine/, i.e. the cyber-engine root.
target_include_directories(cyberengine_jni PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${JNI_ROOT}
)

target_compile_options(cyberengine_jni PRIVATE
    -Wall -Wextra -Wno-unused-parameter -O2 -fvisibility=hidden -fexceptions
)
target_compile_definitions(cyberengine_jni PRIVATE
    CYBER_ANDROID=1
    LOG_TAG="CyberEngine"
)

# Link Android platform libs
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv3)
find_library(log-lib log)
target_link_libraries(cyberengine_jni
    ${android-lib}
    ${egl-lib}
    ${gles-lib}
    ${log-lib}
)
