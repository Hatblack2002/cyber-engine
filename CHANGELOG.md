# CHANGELOG

All notable changes to Cyber Engine. Dates are UTC.
This file is a factual log of what was implemented and verified — no claims of "works" without verification.

---

## 2026-09-22 — Snapshot for handoff

### Final state at snapshot
- Host tests: 53/53 PASS
- APK: `release/CyberEngine-current.apk` (3,220,478 bytes)
- SHA-256: `70bc4cffe8f9856aa88d4b1c6aa66c4acccaebd10a05a7ef8e4a50495897892f`
- ABIs: arm64-v8a only
- No device verification in this session

### KNOWN_BUGS at snapshot
- BUG #1: Multi-line PASTE concatenates lines instead of preserving them (UNRESOLVED)
- BUG #2: `clear` shell command does not actually clear the terminal (MINOR)
- BUG #3: Boot diagnostics verbose on every session start (MINOR)

---

## 2026-09-22 — Multi-line paste fix attempt

### Added
- `android/app/src/main/java/com/cyberengine/app/CyberEditText.kt` — Custom EditText that overrides `onTextContextMenuItem(paste)` to set `isPasteOperation = true` flag.
- TextWatcher in `TerminalView.kt` now checks `inputView.isPasteOperation` and skips dispatch when true.
- `dispatchEnter()` now extracts the FIRST line (text before first `\n`) as `cmd` and keeps the rest in the EditText for subsequent Enters.
- `isSingleLine = false` on inputView to allow multi-line text preservation.

### Result on physical device
- BUG NOT FIXED — IME strips `\n` from clipboard paste, so multi-line content arrives concatenated.
- Single-line paste + Enter still works.
- Keyboard Enter still works.

### Files changed
- `android/app/src/main/java/com/cyberengine/app/CyberEditText.kt` (NEW)
- `android/app/src/main/java/com/cyberengine/app/TerminalView.kt` (rewritten)

---

## 2026-09-22 — TerminalSession / TerminalView split + rotation persistence

### Added
- `android/app/src/main/java/com/cyberengine/app/TerminalSession.kt` — Owns PTY session, output buffer, ANSI parser, read loop. Survives rotation via `AtomicReference<TerminalView?>`.
- `TerminalView` refactored to thin renderer + input handler that ATTACHES to a session.
- `MainActivity` owns `session: TerminalSession?` as a property (survives `onConfigurationChanged`).
- `onConfigurationChanged` now: detach old view → re-inflate layout → attach new view to SAME session. Does NOT call `startSession` again.

### Verified on physical device (earlier session)
- PTY real, execve, BusyBox ARM64, ash all work.
- `echo`, `pwd`, `ls`, `mkdir`, `cd`, `cat`, `rm`, `exit` all execute correctly.
- Filesystem changes verified (files created, removed).

### Files changed
- `android/app/src/main/java/com/cyberengine/app/TerminalSession.kt` (NEW)
- `android/app/src/main/java/com/cyberengine/app/TerminalView.kt` (rewritten)
- `android/app/src/main/java/com/cyberengine/app/MainActivity.kt` (rewritten)

---

## 2026-09-22 — PTY ECHO disabled + manual echo in UI

### Changed
- `android/jni/linux_bridge.cpp` — Added termios manipulation in child after dup2 and before execve:
  ```c
  tcgetattr(0, &tio);
  tio.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL);
  tio.c_lflag |= (ICANON | ISIG);
  tio.c_iflag |= ICRNL;
  tcsetattr(0, TCSANOW, &tio);
  ```
- Manual echo of `cmd + "\r\n"` now done in TerminalView.dispatchEnter BEFORE commandHandler runs.

### Reason
- Previous code had duplicate input display (EditText + PTY echo).
- Disabling PTY echo ensures the input appears exactly once (in the buffer via manual echo).

---

## 2026-09-22 — BusyBox moved from asset to native lib

### Changed
- `libbusybox.so` moved from `assets/linux/busybox` to `jniLibs/arm64-v8a/libbusybox.so`.
- `AndroidManifest.xml` — Added `android:extractNativeLibs="true"`.
- `android/app/build.gradle.kts` — Added `packaging.jniLibs.useLegacyPackaging = true`.
- `UserlandInstaller.kt` — `busyboxPath` now resolves to `applicationInfo.nativeLibraryDir + "/libbusybox.so"`.

### Reason
- Previous location (`/data/data/<package>/files/linux/bin/busybox`) had the `app_data_file` SELinux label with W^X (write xor execute) denied since Android 10.
- execve() returned ENOEXEC or EACCES → exit status 32512 (0x7F00 = exit code 127 = "exec failed").
- New location (`/data/app/<package>/lib/arm64-v8a/libbusybox.so`) has `apk_data_file` label and IS executable.

### Verified on physical device
- After fix, `echo hello`, `pwd`, `ls`, etc. all execute correctly.
- BusyBox ash starts and prints prompt.

---

## 2026-09-22 — Adaptive orientation (portrait + landscape)

### Changed
- `AndroidManifest.xml` — Removed `android:screenOrientation="portrait"`.
- Added `android:configChanges="orientation|screenSize|keyboardHidden|keyboard|screenLayout"`.
- `android/app/src/main/res/layout-land/main.xml` — NEW landscape layout (3-column grid: NAV | VIEWPORT+TERMINAL+AUTOMATION | SYSTEM+CAPS+NETWORK+BOOT).
- `MainActivity.onConfigurationChanged` re-inflates layout and re-binds views.

### Verified on device
- NOT verified on physical device in current session.
- Code compiles; both layouts are present in APK.

---

## 2026-09-22 — Boot diagnostics system

### Added
- `engine/diagnostics/BootDiagnostics.hpp` — Ring buffer of boot stages with `[BOOT]`/`[ERROR]` markers.
- `android/jni/cyber_jni.cpp` — Boot marks at every stage: JNILibraryLoaded, EngineCreated, RendererCreationStarted, EGLInitialization, GLESContextCreated, SurfaceCreated, LinuxBridgeInitialized, BusyBoxLocated, PTYInitialized, ShellSessionStarted.
- New JNI exports: `nativeBootLog(handle)`, `nativeBootMark(handle, stage, ok, reason)`.
- BOOT panel in UI (visible via "BOOT" sidebar tap).

---

## 2026-09-22 — Linux bridge diagnostics per stage

### Added
- `linux_bridge.cpp` — Every PTY step now logs via `emitf(2, ...)`:
  - `posix_openpt` / `grantpt` / `unlockpt` / `ptsname` / `open(slave)` / `fork` (parent side)
  - `setsid` / `TIOCSCTTY` / `dup2(stdin)` / `dup2(stdout)` / `dup2(stderr)` / `chdir` / `execve` (child side)
- Pre-flight `stat()` on busybox path before fork.
- `[PTY][EXEC_ERROR] path=... errno=N message=...` if execve fails.

---

## 2026-09-21 — Capabilities registered honestly

### Added
- `AppPrivateStorage` = `AVAILABLE`
- `SharedStorage` = `REQUIRES_PERMISSION`
- `SAF` = `REQUIRES_PERMISSION` (not yet wired)
- `Filesystem` = `PARTIAL` (app-private only)
- `SystemInfo` = `AVAILABLE` (RAM, battery, CPU count, storage via StatFs)
- `Terminal` = `PARTIAL` (pending device verification)
- `CPU %` = `NOT_IMPLEMENTED` (no reliable API)
- `Temperature` = `NOT_IMPLEMENTED` (no stable API)

---

## 2026-09-21 — Cross-compiled BusyBox 1.36.1 static for arm64

### Added
- `userland/build_busybox.sh` — Script that:
  1. `make defconfig` + `make olddefconfig` via NDK r27 clang
  2. Force-set `CONFIG_STATIC=y` + core applets
  3. Disable applets that fail on bionic (HOSTID, LOADFONT, SU, ADJTIMEX, ETHER_WAKE, RUNLEVEL, etc.)
  4. Regenerate autoconf.h non-interactively via `script -q -c "..." /dev/null` pty trick with `yes "" | head -2000` feeding defaults
  5. Build with `EXTRA_LDFLAGS="-static -pie -Wl,--allow-multiple-definition"`
- `userland/binaries/busybox` — 774 KB ELF arm64 static, verified with `file` and `llvm-readelf`.

### Disabled applets (don't compile against bionic)
- HOSTID (no `gethostid` in bionic)
- LOADFONT/SETFONT/SHOWKEY/DUMPKMAP/KBD_MODE/OPENVT/RESIZE/CONSPY (no `sys/kd.h`)
- RUNLEVEL (no `utmpxname`)
- SU/SULOGIN/PASSWD/ADJTIMEX/ETHER_WAKE/ARP/ARPING
- GETTY/INIT/HALT/REBOOT/POWEROFF/SUSPEND
- HTTPD/INETD/TELNET/TELNETD/TFTPD/TFTP/FTPGET/FTPPUT/DNSD/NSLOOKUP/NTPD/TCPSVD/UDPSVD
- CHROOT/RTCWAKE/RUNIT/RUNSV/RUNSVDIR/SVC/CHPST/WALL/CHAT
- START_STOP_DAEMON/CTTYHACK/FALLOCATE/FATATTR/FSTRIM/SETSID/UNSHARE
- MTDUTILS/MTDWRITE/FLASH_*/UBIATTACH/UBIDETACH/UBI*/NBDCLIENT
- CROND/CRONTAB/AT/RDATE/UPTIME/UPTIME_ONE_LINE/DSLM
- MKFS_MINIX/MKFS_EXT2/MKFS_VFAT/MKFS_REISERFS/SWAPONOFF/MKSWAP
- VOLUMEID/FEATURE_MOUNT_LABEL/MTAB_SUPPORT/HDPARM
- HUSH/FEATURE_INDIVIDUAL/FEATURE_SUID/FEATURE_PREFER_APPLETS
- FEATURE_SH_STANDALONE/FEATURE_SH_NOFORK
- FEATURE_BUFFERS_USE_MALLOC/FEATURE_BUFFERS_GO_ON_STACK/FEATURE_BUFFERS_GO_IN_BSS
- SELINUX/LOCALE_SUPPORT/FEATURE_USE_TERMIOS/LOADKEYS/SETKEYCODES

---

## 2026-09-21 — Engine core + GLES renderer + tests

### Added
- `engine/core/Types.hpp` — Result<T>, Status, IDs, Handles, Span, StringView
- `engine/memory/SystemAllocator.hpp` — malloc/free wrapper with counters (uses posix_memalign for over-aligned allocs)
- `engine/diagnostics/Diagnostics.hpp` — Ring buffer with steady_clock timestamps
- `engine/jobs/JobSystem.hpp` — Worker pool with std::thread + condition_variable
- `engine/events/EventBus.hpp` — Type-erased pub/sub via std::type_index + std::any
- `engine/capabilities/CapabilityManager.hpp` — Singleton registry with mutex
- `engine/Engine.hpp` / `.cpp` — Lifecycle: create/destroy/pause/resume/tick/render
- `engine/shell/Shell.hpp` / `.cpp` — Tokenizer with single/double quotes; built-in commands
- `engine/scene/Scene.hpp` / `.cpp` — Mat4 math, Camera (orbit/pan/zoom), cube MVP
- `engine/renderer/IRenderer.hpp` — Abstract interface + factory
- `engine/renderer/HostStub.cpp` — Returns nullptr for host builds (no EGL)
- `engine/renderer/gles/GLESRenderer.hpp` / `.cpp` — Real EGL+GLES3 backend with cube mesh + per-vertex color shader
- `tests/unit/test_main.cpp` — 10 suites, 53 checks (all pass on host)
- `tools/host_shell.cpp` — CLI demo of engine + shell

---

## 2026-09-21 — Environment setup

### Installed (user-space, no root)
- CMake 3.30.5 (portable binary)
- Ninja 1.12.1 (portable binary)
- Android SDK cmdline-tools 12.0
- Android SDK platform-tools, build-tools 34.0.0, platforms;android-34, ndk;27.2.12479018, cmake;3.30.5
- Gradle 8.7 (binary distribution)
- Java 21 (already present)

### Environment script
- `/home/z/.android-env.sh` — Sets `ANDROID_HOME`, `ANDROID_NDK_HOME`, `GRADLE_HOME`, `CMAKE_HOME`, `PATH`

---

## 2026-09-21 — Project bootstrap

### Added
- Project structure at `/home/z/my-project/cyber-engine/`
- Top-level `CMakeLists.txt` for host build
- `android/` Gradle project with AGP 8.5.2 + Kotlin 1.9.24
- `android/jni/` JNI bridge sources
- `android/app/src/main/` AndroidManifest + layouts + resources
- `userland/` BusyBox build script + cross-compiled binary
- `docs/SPEC_ADAPTADA_AGENTE_v1.1.md` — Inherited spec from user
