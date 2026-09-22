# Cyber Engine — Project Structure

Snapshot date: 2026-09-22
Project root: `/home/z/my-project/cyber-engine/`

## Directory tree

```
cyber-engine/
├── CMakeLists.txt                  # Top-level CMake (engine host build + tests)
├── MANIFEST_SHA256.txt             # SHA-256 hashes of all source files
├── PROJECT_STRUCTURE.md            # This file
├── MASTER_SPEC.md                  # Conceptual + technical spec
├── PROJECT_STATUS.md               # IMPLEMENTED / PARTIAL / NOT_IMPLEMENTED / KNOWN_BUGS / UNVERIFIED
├── CHANGELOG.md                    # Chronological change log
├── HANDOFF.md                      # Instructions for the next agent
├── RELEASE_MANIFEST.md             # Release snapshot manifest
│
├── engine/                         # C++20 engine core (host-buildable + Android target)
│   ├── Engine.hpp / Engine.cpp     # Lifecycle: create/destroy/pause/resume/tick/render
│   ├── core/Types.hpp              # Result<T>, Status, IDs, Handles, Span, StringView
│   ├── memory/SystemAllocator.hpp  # malloc/free wrapper + alloc counters
│   ├── diagnostics/
│   │   ├── Diagnostics.hpp        # Ring buffer for runtime logs
│   │   └── BootDiagnostics.hpp    # Boot stage tracker [BOOT]/[ERROR]
│   ├── jobs/JobSystem.hpp          # Worker pool (no P01 deps)
│   ├── events/EventBus.hpp        # Typed pub/sub (type-erased)
│   ├── capabilities/CapabilityManager.hpp  # Single source of truth for capabilities
│   ├── renderer/
│   │   ├── IRenderer.hpp          # Abstract renderer + factory
│   │   ├── HostStub.cpp           # nullptr renderer for host builds
│   │   └── gles/
│   │       ├── GLESRenderer.hpp
│   │       └── GLESRenderer.cpp   # Real EGL+GLES3 backend (Android only)
│   ├── scene/
│   │   ├── Scene.hpp
│   │   └── Scene.cpp              # Camera (orbit/pan/zoom) + cube mesh + MVP
│   └── shell/
│       ├── Shell.hpp
│       └── Shell.cpp              # help/version/capabilities/diagnostics/echo/clear
│
├── android/                        # Android app module (Gradle + NDK + Kotlin)
│   ├── build.gradle.kts            # Top-level Gradle config (AGP 8.5.2, Kotlin 1.9.24)
│   ├── settings.gradle.kts
│   ├── gradle.properties
│   ├── local.properties            # SDK + NDK absolute paths (machine-specific)
│   ├── jni/                        # JNI bridge sources
│   │   ├── cyber_jni.hpp
│   │   ├── cyber_jni.cpp           # Engine + boot diagnostics + surface + touch
│   │   ├── jni_state.hpp           # Singleton slots for renderer/scene
│   │   └── linux_bridge.cpp         # PTY + fork + setsid + dup2 + execve (busybox ash)
│   └── app/                        # Application module
│       ├── build.gradle.kts        # arm64-v8a, minSdk 26, targetSdk 34, extractNativeLibs=true
│       ├── CMakeLists.txt          # NDK build → libcyberengine_jni.so
│       └── src/main/
│           ├── AndroidManifest.xml  # No screenOrientation lock, configChanges for rotation
│           ├── jniLibs/arm64-v8a/libbusybox.so  # BusyBox as packaged native lib (executable)
│           ├── assets/linux/profile.sh            # Shell environment setup
│           ├── java/com/cyberengine/app/
│           │   ├── MainActivity.kt         # Activity, owns TerminalSession, rotation handling
│           │   ├── CyberBridge.kt          # JNI surface (25 external fun)
│           │   ├── CyberEditText.kt       # Custom EditText that detects PASTE (isPasteOperation flag)
│           │   ├── CyberSurfaceView.kt     # SurfaceView wrapper for GLES
│           │   ├── TerminalSession.kt      # PTY owner + buffer + read loop (survives rotation)
│           │   ├── TerminalView.kt         # Thin renderer + input handler (recreated on rotation)
│           │   └── UserlandInstaller.kt    # Extracts busybox from native lib dir, installs applets
│           └── res/
│               ├── layout/main.xml          # Portrait layout
│               ├── layout-land/main.xml     # Landscape layout (3-column grid)
│               └── values/
│                   ├── colors.xml           # Cyberpunk palette
│                   ├── strings.xml
│                   └── themes.xml           # Theme.DeviceDefault.NoActionBar base
│
├── tests/
│   ├── CMakeLists.txt
│   └── unit/test_main.cpp          # 10 suites, 53 checks (host)
│
├── tools/
│   ├── CMakeLists.txt
│   └── host_shell.cpp              # CLI to exercise the engine headless
│
├── userland/
│   ├── build_busybox.sh             # Cross-compile script for busybox 1.36.1 (arm64 static)
│   ├── binaries/busybox             # Cross-compiled ARM64 static binary (774 KB)
│   └── src/                          # Busybox 1.36.1 + Bash 5.2.15 sources (not in repo export)
│
├── docs/
│   └── SPEC_ADAPTADA_AGENTE_v1.1.md  # Original spec inherited from the user
│
├── release/
│   ├── CyberEngine-current.apk       # Final debug-signed APK (3.07 MB)
│   └── busybox-arm64                 # ARM64 static busybox binary (774 KB)
│
└── build-host/                      # Host build artifacts (not in export)
```

## Build outputs (not in repo export)

| Path | Contents |
|---|---|
| `build-host/` | CMake build for host tests (`libcyberengine.a`, `cyber_tests`, `cyber_shell`) |
| `android/app/build/` | Gradle build outputs (intermediate + final APK) |
| `android/app/.cxx/` | CMake/NDK intermediate object files |

These are regenerated from source by the build commands in `BUILD.md` (this snapshot — see HANDOFF.md section "How to build").

## Important notes

- `android/local.properties` contains absolute paths to `/home/z/android-sdk` — **machine-specific**. The next agent must update `sdk.dir` to point at their local Android SDK install.
- `userland/src/` is NOT included in the snapshot (large, regenerable via `userland/build_busybox.sh`).
- `engine/renderer/gles/GLESRenderer.cpp` is excluded from host build via `#if defined(__ANDROID__)`. It only compiles under the NDK.
- The busybox binary in `android/app/src/main/jniLibs/arm64-v8a/libbusybox.so` is an ELF executable disguised with a `.so` extension so Android's installer extracts it to the executable native lib dir.
