# Cyber Engine

A Android-native cyberpunk terminal/dashboard application with its OWN Linux userland (no Termux dependency).

![Status: Development snapshot](https://img.shields.io/badge/status-snapshot-orange)
![Tests: 53/53 host](https://img.shields.io/badge/tests-53%2F53%20host-brightgreen)
![APK: 3.07 MB](https://img.shields.io/badge/APK-3.07%20MB-blue)
![ABI: arm64-v8a](https://img.shields.io/badge/ABI-arm64--v8a-lightgrey)

---

## What this is

This repository is a **development snapshot** of Cyber Engine, ready for handoff to another agent.

It is NOT a finished product. It is a checkpoint with full source code, build verification, and honest documentation of what works vs what doesn't.

The current critical bug: **multi-line PASTE in the terminal concatenates lines instead of preserving them**. See `PROJECT_STATUS.md` §KNOWN_BUGS.

---

## Documentation map

| Document | Read it to understand |
|---|---|
| `MASTER_SPEC.md` | The non-negotiable rules (anti-simulation, architecture, capability honesty) |
| `PROJECT_STATUS.md` | What's actually IMPLEMENTED vs PARTIAL vs NOT_IMPLEMENTED vs KNOWN_BUGS vs UNVERIFIED |
| `PROJECT_STRUCTURE.md` | The file tree and what each directory contains |
| `HANDOFF.md` | What the next agent MUST do before writing any code |
| `CHANGELOG.md` | Chronological log of changes |
| `RELEASE_MANIFEST.md` | The current release artifact details |
| `MANIFEST_SHA256.txt` | Integrity checksums |
| `docs/SPEC_ADAPTADA_AGENTE_v1.1.md` | The original spec inherited from the user |

---

## Quick start (build + test)

### Prerequisites

- CMake >= 3.18
- Ninja
- Java 17 or 21
- Android SDK with: cmdline-tools, platform-tools, build-tools;34.0.0, platforms;android-34, ndk;27.2.12479018, cmake;3.30.5
- Gradle 8.7

### Update `android/local.properties` with your paths

```
sdk.dir=/path/to/your/android-sdk
ndk.dir=/path/to/your/android-sdk/ndk/27.2.12479018
```

### Host tests

```
cd /path/to/cyber-engine
rm -rf build-host && mkdir build-host && cd build-host
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j2
./tests/cyber_tests
# Expected: 53 passed, 0 failed
```

### Build APK

```
cd /path/to/cyber-engine/android
gradle :app:assembleDebug --console=plain
# Output: app/build/outputs/apk/debug/app-debug.apk
```

### Verify APK

```
APK=app/build/outputs/apk/debug/app-debug.apk
unzip -l $APK | grep -E "\.so$|assets/"
# Expected:
#   lib/arm64-v8a/libbusybox.so
#   lib/arm64-v8a/libcyberengine_jni.so
#   assets/linux/profile.sh
```

### Install on device

```
adb install -r release/CyberEngine-current.apk
adb shell am start -n com.cyberengine.app.debug/com.cyberengine.app.MainActivity
```

---

## Architecture overview

```
Android APK
    ↓
MainActivity (Kotlin) — owns TerminalSession, handles rotation
    ↓
CyberBridge (JNI surface — 25 external fun)
    ↓
C++ Engine (Result/Status lifecycle, CapabilityManager, Shell, Scene, Renderer)
    ↓
GLES3 + EGL backend (real, NOT Vulkan)
    ↓
Linux Bridge (PTY: posix_openpt + fork + setsid + dup2 + execve)
    ↓
BusyBox 1.36.1 (ARM64 static, cross-compiled with NDK r27 — bundled as libbusybox.so)
    ↓
ash shell
```

The application does NOT depend on Termux being installed. The Linux userland is bundled inside the APK as a native library (`libbusybox.so` in `jniLibs/arm64-v8a/`).

---

## Anti-simulation rule

**NO CAPABILITY MAY BE SIMULATED.**

If a capability cannot be demonstrated on a real device, it is marked `NOT_IMPLEMENTED` or `UNVERIFIED` — never `IMPLEMENTED`. See `MASTER_SPEC.md` §2 for the full rule.

Forbidden simulations: fake root, fake network scanning, fake process listings, fake filesystem contents, fake FPS, fake metrics, fake shell output.

---

## License

This is a private development project. No license granted for redistribution.

## Acknowledgements

- BusyBox 1.36.1 (GPLv2) — https://busybox.net/
- Android NDK r27 — https://developer.android.com/ndk
- The original spec was provided by the project owner.
