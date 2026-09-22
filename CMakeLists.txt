# cyber-engine :: engine library CMake
cmake_minimum_required(VERSION 3.18)
project(cyberengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Engine core library — header-mostly + Engine.cpp + Shell.cpp + Scene.cpp
add_library(cyberengine STATIC
    engine/Engine.cpp
    engine/shell/Shell.cpp
    engine/scene/Scene.cpp
    engine/renderer/HostStub.cpp
)
target_include_directories(cyberengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(cyberengine PUBLIC cxx_std_20)
target_compile_options(cyberengine PRIVATE -Wall -Wextra -Wno-unused-parameter)

# Android-only: real GLES renderer.
if(ANDROID)
    target_sources(cyberengine PRIVATE
        engine/renderer/gles/GLESRenderer.cpp
    )
    target_include_directories(cyberengine PRIVATE
        ${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include
    )
    find_library(android-lib android)
    find_library(egl-lib EGL)
    find_library(gles-lib GLESv3)
    find_library(log-lib log)
    target_link_libraries(cyberengine PUBLIC ${android-lib} ${egl-lib} ${gles-lib} ${log-lib})
    target_compile_definitions(cyberengine PUBLIC CYBER_ANDROID=1)
    # Drop the host stub on Android.
    set_source_files_properties(engine/renderer/HostStub.cpp PROPERTIES HEADER_FILE_ONLY ON)
endif()

# Host tests target.
if(NOT ANDROID)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(tools)
endif()
