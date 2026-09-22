# Cyber Engine — Master Specification

**Status:** INHERITED + EXTENDED from `docs/SPEC_ADAPTADA_AGENTE_v1.1.md`
**Last revision:** 2026-09-22
**Scope:** This document is the single source of truth for what the project MUST be. Any code that contradicts this spec is a bug.

---

## 1. Vision

Cyber Engine is an Android-native cyberpunk-styled terminal/dashboard application that ships with its OWN Linux userland (no Termux dependency). The product is a real, honest engine — never a mockup. The visual aesthetic is a layer; the operational reality is the base.

```
Android APK → Activity → JNI → C++ Engine → EGL/GLES3 → Render Loop
                                     ↓
                              Linux userland propio (BusyBox ARM64)
                                     ↓
                                    PTY
                                     ↓
                                  BusyBox → ash
```

---

## 2. Anti-simulation rule (NON-NEGOTIABLE)

**NO CAPABILITY MAY BE SIMULATED.**

If something cannot be demonstrated in code on the real device, it MUST be reported as `NOT_IMPLEMENTED` or `UNVERIFIED` — never `IMPLEMENTED`.

Forbidden simulations:
- Fake root detection
- Fake network scanning / ARP / nmap / packet capture
- Fake process listings
- Fake filesystem contents
- Fake FPS counter (must reflect actual frames)
- Fake CPU/RAM/battery metrics
- Fake shell command output

A capability that exists in code but was not verified on a physical Android device MUST be marked `UNVERIFIED` in `PROJECT_STATUS.md` and `PARTIAL` in the runtime `CapabilityManager`.

---

## 3. Architecture (INVARIABLE)

```
Core → Memory → Jobs → EventBus → CapabilityManager → Renderer → Scene → UI
```

Specialized modules connect ONLY through contracts or EventBus: Filesystem, Process, Network, AndroidBridge, Lua, Shell.

**Forbidden couplings:**
- UI → Vulkan internals
- Lua → private registries
- Core → Android Activity

Boot sequence (each stage returns `Result` — no `tryInit(); ignoreFailure()`):

```
Activity → AndroidBridge.initialize() → Engine::create()
  → Diagnostics → Memory → JobSystem → EventBus → CapabilityManager
  → Renderer (Vulkan | GLES | fail) → Scene → UI → Lua → READY
```

---

## 4. Linux userland — OWN bridge, no Termux

The application MUST NOT depend on Termux being installed.

Architecture:

```
Cyber Command OS (Kotlin)
        ↓ JNI
   C++ Linux Bridge (android/jni/linux_bridge.cpp)
        ↓ posix_openpt + fork + setsid + dup2 + execve
       PTY
        ↓
   BusyBox ARM64 (libbusybox.so packaged as native lib)
        ↓
        ash
```

### 4.1 Userland location

- **Executable (busybox):** `/data/app/<package>/lib/arm64-v8a/libbusybox.so` — Android extracts the `.so` to this directory at install time (because `android:extractNativeLibs="true"`). This directory has the `apk_data_file` SELinux label and IS EXECUTABLE.
- **PREFIX tree (HOME, TMPDIR, etc.):** `/data/data/<package>/files/linux/` — directory structure: `bin/`, `sbin/`, `usr/bin/`, `usr/sbin/`, `home/`, `tmp/`, `etc/`, `var/`, `var/log/`. Writable but NOT executable (W^X since Android 10) — used for HOME/TMPDIR only.

### 4.2 Environment variables (set by linux_bridge.cpp child)

| Variable | Value |
|---|---|
| `PATH` | `$PREFIX/bin:$PREFIX/sbin:$PREFIX/usr/bin:/system/bin:/system/xbin` |
| `HOME` | `$PREFIX/home` |
| `TMPDIR` | `$PREFIX/tmp` |
| `TERM` | `xterm-256color` |
| `LANG` | `C.UTF-8` |
| `LC_ALL` | `C.UTF-8` |
| `CYBER_ENGINE` | `1` |
| `CYBER_USERLAND` | `$PREFIX` |
| `PS1` | `cyber# ` |

### 4.3 PTY termios configuration (child, before execve)

```
tcgetattr(0, &tio);
tio.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL);  // disable ECHO
tio.c_lflag |= (ICANON | ISIG);                              // line-buffered + signals
tio.c_iflag |= ICRNL;                                        // CR → NL
tcsetattr(0, TCSANOW, &tio);
```

The UI performs MANUAL ECHO of the user's typed command. PTY does NOT echo input back.

---

## 5. Orientation — adaptive PORTRAIT + LANDSCAPE

The application MUST support both portrait and landscape. It MUST NOT lock to portrait.

- `AndroidManifest.xml` MUST NOT contain `android:screenOrientation="..."`.
- `<activity>` MUST have `android:configChanges="orientation|screenSize|keyboardHidden|keyboard|screenLayout"` so the Activity is NOT recreated on rotation.
- `MainActivity.onConfigurationChanged()` MUST re-inflate the layout (`setContentView(R.layout.main)` — Android picks `layout/main.xml` or `layout-land/main.xml` based on orientation).
- The `TerminalSession` (PTY + buffer + read loop) MUST survive rotation. Only the `TerminalView` is recreated and re-attached to the same session.

### 5.1 Portrait layout (vertical stack)

```
HEADER
NAVIGATION (horizontal scroll)
SYSTEM
NETWORK (NOT_IMPLEMENTED badge)
3D VIEWPORT
CAPABILITIES (small)
TERMINAL
AUTOMATION (NOT_IMPLEMENTED badge)
BOOT (hidden by default)
```

### 5.2 Landscape layout (3-column grid)

```
┌────────┬──────────────────────┬──────────┐
│ NAV    │       3D VIEW        │ SYSTEM   │
│        │                      │ CAPS     │
│        ├──────────┬───────────┤ NETWORK  │
│        │ TERMINAL │ AUTOMATION│ BOOT     │
└────────┴──────────┴───────────┴──────────┘
```

---

## 6. TerminalSession persistence

```
TerminalSession (owned by MainActivity, survives rotation)
    ├── PTY session id (from nativeTerminalStart)
    ├── child PID (tracked in C++ via g_sessions[])
    ├── SpannableStringBuilder (output buffer + ANSI spans)
    ├── read loop (single background thread, sole writer of buffer)
    └── attachedView: AtomicReference<TerminalView?>  (weak ref to current view)

TerminalView (transient — recreated on rotation)
    └── attach(session) / detach()
        ├── renders session.buffer via outputView.setText
        └── forwards input via session.write / session.sendSignal
```

Rules:
- `onConfigurationChanged` MUST detach the old view, re-inflate layout, attach the new view to the SAME session.
- `terminalView.startSession(...)` MUST NOT be called on rotation — only on first creation in `onCreate`.
- The read loop MUST NOT be registered twice.
- The shell process MUST NOT be killed on rotation.
- Working directory, environment, scrollback MUST all be preserved.

---

## 7. Capability honesty

Capabilities use the canonical state enum:

```
AVAILABLE | PARTIAL | UNAVAILABLE | REQUIRES_PERMISSION | REQUIRES_ROOT |
REQUIRES_HARDWARE | NOT_IMPLEMENTED | ERROR
```

`CapabilityManager` is the SINGLE source of truth. The UI queries it; modules set their own state honestly.

| Capability | Required state in P01 |
|---|---|
| `Renderer.Vulkan` | `NOT_IMPLEMENTED` |
| `Renderer.GLES` | `AVAILABLE` (after first surface created) |
| `Storage.AppPrivate` | `AVAILABLE` |
| `Storage.Shared` | `REQUIRES_PERMISSION` |
| `Storage.SAF` | `REQUIRES_PERMISSION` (not yet wired) |
| `Root` | `UNAVAILABLE` (no fake detection) |
| `Network.Observation` | `NOT_IMPLEMENTED` (spec §0 forbids faking) |
| `Process.Info` | `REQUIRES_ROOT` (no /proc full) |
| `Terminal` | `PARTIAL` until PTY verified on device |
| `Lua.VM` | `NOT_IMPLEMENTED` |
| `NativePlugins` | `UNAVAILABLE` (disabled per spec) |
| `Filesystem` | `PARTIAL` (app-private only) |
| `System.Info` | `AVAILABLE` (RAM, battery, CPU count, storage via StatFs) |
| `Scene3D` | `AVAILABLE` (cube mesh + camera) |
| `Camera` | `AVAILABLE` (orbit/pan/zoom) |
| `Touch` | `AVAILABLE` (1-finger orbit, pinch zoom, 2-finger pan) |
| `Picking` | `NOT_IMPLEMENTED` |
| `Shell` | `AVAILABLE` (built-in cyber shell) |
| `UI.Dashboard` | `AVAILABLE` |

---

## 8. UI rules

The main dashboard MUST NOT show implementation details to the user. Specifically, these MUST NOT appear in the main dashboard view:

- GLES / EGL / JNI / PTY / BusyBox / ABI / internal file paths / frame count / internal logs

These CAN appear ONLY in:
- Diagnostics panel (when user opens it)
- Boot log panel (when user opens it)
- System Info panel (only what real Android APIs provide)

The terminal MUST look like a real Linux terminal — `cyber#` prompt, real command output, real error messages.

### 8.1 Terminal input rules (CURRENT SPEC — partially unimplemented, see KNOWN_BUGS)

- Keyboard Enter: fires `dispatchEnter()` exactly ONCE per Enter press.
- Single-line paste: do NOT auto-execute; user must press Enter.
- Multi-line paste: do NOT auto-execute; preserve full content in EditText; user presses Enter to execute one line at a time.
- `dispatchEnter` extracts the FIRST line (text before first `\n`) as `cmd`, keeps the rest in the EditText.
- Manual echo: `session.appendOutput(cmd + "\r\n")` (because PTY ECHO is disabled in termios).
- One PTY write per Enter: `session.write((cmd + "\r").toByteArray())`.

---

## 9. Diagnostics rule

Every boot stage MUST be recorded in `BootDiagnostics` (ring buffer). Format:

```
[BOOT] StageName   reason=...     (success)
[ERROR] StageName  reason=...     (failure)
```

If a stage fails, the error MUST be visible — never hidden behind an empty screen. The boot log is queryable via `nativeBootLog(handle)` and displayed in the BOOT panel.

Stages (in order):
1. `ActivityCreated`
2. `JNILibraryLoaded`
3. `EngineCreated`
4. `RendererCreationStarted`
5. `EGLInitialization`
6. `GLESContextCreated`
7. `SurfaceCreated`
8. `RenderLoopStarted`
9. `LinuxBridgeInitialized`
10. `BusyBoxLocated`
11. `PTYInitialized`
12. `ShellSessionStarted`

---

## 10. What this spec does NOT require (out of scope for P01)

The following are deferred and MUST NOT be implemented in this snapshot's scope:

- Vulkan backend
- Lua VM + sandbox
- Material system (albedo, roughness, metalness)
- GLB/GLTF asset loader
- Picking (NDC ray)
- World map / cyberpunk map visualization
- Visual effects (bloom, glow, scanlines — except what's in the existing layout colors)
- Network observation of any kind
- Process listing
- CPU % meter
- Temperature sensor reading

These remain `NOT_IMPLEMENTED` per spec §0.

---

## 11. Definition of "IMPLEMENTED" (spec §51)

A capability is `IMPLEMENTED` only if ALL of:

- compiles
- runs in operation
- uses real data
- handles errors
- has tests OR an acceptance criterion
- has ownership
- respects threading
- registers with `CapabilityManager`
- **has no fake implementation behind the interface**

If any condition fails, it is `NOT_IMPLEMENTED` or `UNVERIFIED`.
