# RELEASE MANIFEST — Cyber Engine

**Release date:** 2026-09-22
**Release type:** Development snapshot for handoff (debug-signed)
**Repository root:** `/home/z/my-project/cyber-engine/`

---

## Artifact

| Item | Value |
|---|---|
| APK path | `release/CyberEngine-current.apk` |
| APK size | 3,220,478 bytes (3.07 MB) |
| APK SHA-256 | `70bc4cffe8f9856aa88d4b1c6aa66c4acccaebd10a05a7ef8e4a50495897892f` |
| Package | `com.cyberengine.app.debug` |
| Version name | `0.1.0-debug` |
| Version code | 3 |
| minSdk | 26 |
| targetSdk | 34 |
| compileSdk | 34 |
| NDK version | 27.2.12479018 |
| Build tools | 34.0.0 |
| Gradle | 8.7 |
| AGP | 8.5.2 |
| Kotlin | 1.9.24 |
| Java | 21 |
| ABIs | `arm64-v8a` (only) |
| Signing | Debug (auto-generated keystore) |

---

## Native libraries inside APK

| Library | Size | Source |
|---|---|---|
| `lib/arm64-v8a/libcyberengine_jni.so` | 1,102,280 bytes | C++ engine + JNI bridge + linux_bridge (this repo) |
| `lib/arm64-v8a/libbusybox.so` | 774,376 bytes | Cross-compiled from busybox-1.36.1 source with NDK r27 |

---

## Assets

| Asset | Size | Purpose |
|---|---|---|
| `assets/linux/profile.sh` | 700 bytes | Shell environment setup (sourced by ash --login) |

---

## Test results (host)

```
$ cd build-host && ./tests/cyber_tests
cyber-engine host tests: 53 passed, 0 failed
```

Suites: 10 (ResultStatus, IDsHandles, Memory, Diagnostics, Jobs, EventBus, Capabilities, Engine, Shell, Scene)
Total checks: 53
Pass: 53 / Fail: 0

No Android instrumented tests exist.
No JNI tests exist.
No UI tests exist.

---

## Build verification

```
$ cd android && gradle :app:assembleDebug --console=plain
BUILD SUCCESSFUL in 26s
39 actionable tasks: 7 executed, 32 up-to-date
```

```
$ cd .. && rm -rf build-host && mkdir build-host && cd build-host
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
$ cmake --build . -j2
[9/9] Linking CXX executable tests/cyber_tests
```

---

## JNI exports

25 `Java_com_cyberengine_app_CyberBridge_*` symbols — matches the 25 `external fun` declarations in `CyberBridge.kt` exactly.

```
Java_com_cyberengine_app_CyberBridge_nativeBootLog
Java_com_cyberengine_app_CyberBridge_nativeBootMark
Java_com_cyberengine_app_CyberBridge_nativeCapabilities
Java_com_cyberengine_app_CyberBridge_nativeDiagnostics
Java_com_cyberengine_app_CyberBridge_nativeEngineCreate
Java_com_cyberengine_app_CyberBridge_nativeEngineDestroy
Java_com_cyberengine_app_CyberBridge_nativeOnPause
Java_com_cyberengine_app_CyberBridge_nativeOnResume
Java_com_cyberengine_app_CyberBridge_nativeOnSurfaceCreated
Java_com_cyberengine_app_CyberBridge_nativeOnSurfaceDestroyed
Java_com_cyberengine_app_CyberBridge_nativeOnSurfaceResized
Java_com_cyberengine_app_CyberBridge_nativeOrbitCamera
Java_com_cyberengine_app_CyberBridge_nativePanCamera
Java_com_cyberengine_app_CyberBridge_nativeRender
Java_com_cyberengine_app_CyberBridge_nativeRendererBackend
Java_com_cyberengine_app_CyberBridge_nativeRendererFrameCount
Java_com_cyberengine_app_CyberBridge_nativeShellExecute
Java_com_cyberengine_app_CyberBridge_nativeTerminalExitStatus
Java_com_cyberengine_app_CyberBridge_nativeTerminalRead
Java_com_cyberengine_app_CyberBridge_nativeTerminalResize
Java_com_cyberengine_app_CyberBridge_nativeTerminalSignal
Java_com_cyberengine_app_CyberBridge_nativeTerminalStart
Java_com_cyberengine_app_CyberBridge_nativeTerminalStop
Java_com_cyberengine_app_CyberBridge_nativeTerminalWrite
Java_com_cyberengine_app_CyberBridge_nativeZoomCamera
```

---

## Manifest state

```xml
<!-- extractNativeLibs=true forces .so extraction to /data/app/<pkg>/lib/<abi>/ -->
<application android:extractNativeLibs="true" ...>
    <activity
        android:name=".MainActivity"
        android:configChanges="orientation|screenSize|keyboardHidden|keyboard|screenLayout"
        <!-- NO screenOrientation — adaptive portrait + landscape -->
        android:windowSoftInputMode="adjustResize">
```

---

## Capabilities at runtime

| Capability | State | Note |
|---|---|---|
| Renderer.Vulkan | NOT_IMPLEMENTED | spec §5 |
| Renderer.GLES | AVAILABLE (after first surface) | UNVERIFIED on device |
| Storage.AppPrivate | AVAILABLE | UNVERIFIED |
| Storage.Shared | REQUIRES_PERMISSION | not wired |
| Storage.SAF | REQUIRES_PERMISSION | not wired |
| Root | UNAVAILABLE | honest "not faking it" |
| Network.Observation | NOT_IMPLEMENTED | spec §0 |
| Process.Info | REQUIRES_ROOT | no /proc full |
| Terminal | PARTIAL | "not yet verified on real device" |
| Lua.VM | NOT_IMPLEMENTED | spec §10 |
| NativePlugins | UNAVAILABLE | spec §6 |
| Filesystem | PARTIAL | app-private only |
| System.Info | AVAILABLE | UNVERIFIED |
| Scene3D | AVAILABLE | UNVERIFIED |
| Camera | AVAILABLE | UNVERIFIED |
| Touch | AVAILABLE | UNVERIFIED |
| Picking | NOT_IMPLEMENTED | spec §9 |
| Shell | AVAILABLE | host-verified only |
| UI.Dashboard | AVAILABLE | UNVERIFIED |

---

## KNOWN_BUGS at this release

1. **Multi-line PASTE concatenates lines instead of preserving them** — UNRESOLVED.
   - Symptom: pasting `echo uno\necho dos\npwd` into the terminal results in `echo unoecho dospwd` (all lines concatenated).
   - Status: previous fix attempts (CyberEditText paste detection, isSingleLine=false) did not resolve the bug on the physical device.
   - Next step: reproduce on device with `adb logcat` to capture the actual text inserted into EditText; identify if IME is stripping `\n` or if EditText is.

2. **`clear` shell command does not clear the terminal** — MINOR.
   - Status: stub returns Ok without UI signal.

3. **Boot diagnostics verbose on every session start** — MINOR.
   - Status: each PTY spawn emits ~12 `[PTY][STEP_OK][CHILD]` lines.

---

## UNVERIFIED at this release (require physical device test)

- GLES3 / EGL render path on real surface
- PTY spawn (posix_openpt + fork + setsid + dup2 + execve)
- BusyBox execution from `/data/app/<package>/lib/arm64-v8a/libbusybox.so`
- Shell session interactive behavior
- stdin/stdout/stderr routing through PTY
- Exit status reporting (waitpid)
- Rotation persistence (TerminalSession survives onConfigurationChanged)
- StatFs storage readout
- BatteryManager readout
- Touch input (orbit, pinch zoom, 2-finger pan)
- Multi-line paste preservation (see KNOWN_BUGS #1)

Previous physical device tests (from earlier session) confirmed:
- PTY real, execve, BusyBox ARM64, ash all work
- `echo`, `pwd`, `ls`, `mkdir`, `cd`, `cat`, `rm`, `exit` all execute correctly
- Filesystem changes verified

But these were on an EARLIER build; the current snapshot's APK has not been re-verified on device.

---

## Application state

The application is NOT finished. It is a development snapshot suitable for handoff to another agent. The receiving agent must:

1. Read `HANDOFF.md` and complete the audit checklist before any code changes.
2. Verify the build on their machine.
3. Re-verify on a physical Android device.
4. Fix KNOWN_BUGS #1 (multi-line paste) before any new feature work.
5. Respect the anti-simulation rule (see `MASTER_SPEC.md` §2).

---

## Git commit

| Field | Value |
|---|---|
| Repository | https://github.com/Hatblack2002/cyber-engine |
| Branch | main |
| Commit SHA (full) | 17e6e2c2c59fc3d81238b09bd49df05ce5bfd63a |
| Commit SHA (short) | 17e6e2c |
| Commit message | "Snapshot: Cyber Engine v0.1.0-debug — handoff to next agent" |
| Commit date (UTC) | 2026-09-22T01:36:10Z |
| Files in repo | 5749 |
| Repo size (.git) | 43M |
