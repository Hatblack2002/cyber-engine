# HANDOFF — For the next agent

## ⚠️ DO NOT MODIFY THE PROJECT YET

Before writing any code or making any changes, you MUST complete this audit checklist. Skipping any step is a critical failure.

---

## Required reading order

1. `MASTER_SPEC.md` — conceptual + technical spec (NON-NEGOTIABLE rules)
2. `PROJECT_STATUS.md` — what's actually IMPLEMENTED / PARTIAL / NOT_IMPLEMENTED / KNOWN_BUGS / UNVERIFIED
3. `HANDOFF.md` — this file
4. `PROJECT_STRUCTURE.md` — file tree
5. `CHANGELOG.md` — what was changed and why
6. `RELEASE_MANIFEST.md` — current release state
7. `MANIFEST_SHA256.txt` — checksums
8. `docs/SPEC_ADAPTADA_AGENTE_v1.1.md` — original spec inherited from the user

---

## Required audit steps (do NOT skip)

### Step 1: Verify the build environment

```
# Required tools (install if missing):
# - CMake >= 3.18
# - Ninja
# - Java 17 or 21
# - Android SDK (cmdline-tools + platform-tools + build-tools;34.0.0 + platforms;android-34 + ndk;27.2.12479018 + cmake;3.30.5)
# - Gradle 8.7

# Update android/local.properties with YOUR paths:
sdk.dir=/path/to/your/android-sdk
ndk.dir=/path/to/your/android-sdk/ndk/27.2.12479018
```

### Step 2: Verify host tests pass

```
cd /path/to/cyber-engine
rm -rf build-host && mkdir build-host && cd build-host
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j2
./tests/cyber_tests
```

**Expected:** `cyber-engine host tests: 53 passed, 0 failed`

If this fails, STOP. The codebase has been corrupted or the environment is wrong. Do not proceed.

### Step 3: Verify APK builds

```
cd /path/to/cyber-engine/android
gradle :app:assembleDebug --console=plain
```

**Expected:** `BUILD SUCCESSFUL`. Output at `app/build/outputs/apk/debug/app-debug.apk`.

### Step 4: Verify APK contents

```
APK=app/build/outputs/apk/debug/app-debug.apk
unzip -l $APK | grep -E "\.so$|assets/"
# Expected:
#   lib/arm64-v8a/libbusybox.so (774376 bytes)
#   lib/arm64-v8a/libcyberengine_jni.so (~1.1 MB)
#   assets/linux/profile.sh (700 bytes)
```

### Step 5: Verify JNI symbol exports match Kotlin declarations

```
unzip -p $APK lib/arm64-v8a/libcyberengine_jni.so > /tmp/lib.so
<NDK>/toolchains/llvm/prebuilt/<host>/bin/llvm-nm -D --defined-only /tmp/lib.so | grep "Java_"
# Count should be 25 — one for each external fun in CyberBridge.kt
grep "external fun" android/app/src/main/java/com/cyberengine/app/CyberBridge.kt | wc -l
# Should also be 25
```

### Step 6: Verify SHA-256 checksums match

```
sha256sum release/CyberEngine-current.apk
# Compare with MANIFEST_SHA256.txt — must match exactly
```

### Step 7: Read the code (no skipping)

Read these files completely:
- `engine/Engine.cpp`
- `engine/shell/Shell.cpp`
- `engine/renderer/gles/GLESRenderer.cpp`
- `android/jni/cyber_jni.cpp`
- `android/jni/linux_bridge.cpp`
- `android/app/src/main/java/com/cyberengine/app/MainActivity.kt`
- `android/app/src/main/java/com/cyberengine/app/TerminalSession.kt`
- `android/app/src/main/java/com/cyberengine/app/TerminalView.kt`
- `android/app/src/main/java/com/cyberengine/app/CyberEditText.kt`
- `android/app/src/main/java/com/cyberengine/app/UserlandInstaller.kt`

### Step 8: Identify discrepancies

Compare what `PROJECT_STATUS.md` says vs what the code actually does. If you find any discrepancy:

- Documentation says `IMPLEMENTED` but code is a stub → mark `NOT_IMPLEMENTED`
- Documentation says `IMPLEMENTED` but no test exists → mark `UNVERIFIED`
- Documentation says `IMPLEMENTED` but was never run on device → mark `UNVERIFIED`

Update `PROJECT_STATUS.md` to reflect the corrected state. Do NOT silently change claims.

### Step 9: Identify KNOWN_BUGS

`PROJECT_STATUS.md` lists 3 known bugs. The most critical is:

**BUG #1 — Multi-line PASTE concatenates lines (UNRESOLVED)**

The previous agent attempted fixes (CyberEditText, isPasteOperation flag, isSingleLine=false) but the bug persists on the physical device. The hypothesis is that the device's IME strips `\n` from clipboard content before inserting into the EditText.

Before attempting any fix, REPRODUCE the bug on the actual device (or a device with the same IME) to confirm the root cause. Do NOT blindly apply a fix without understanding the actual IME behavior.

### Step 10: Produce a diagnostic report

After completing steps 1-9, write a report that includes:
- Environment verification result (pass/fail)
- Host test result (pass count / fail count)
- APK build result (pass/fail)
- JNI symbol count (must be 25)
- SHA-256 match result
- List of discrepancies found in Step 8
- List of KNOWN_BUGS confirmed
- Proposed next steps (DO NOT implement yet — just propose)

ONLY AFTER this report is produced and acknowledged may code changes begin.

---

## ⚠️ ANTI-SIMULATION RULE (NON-NEGOTIABLE)

**NO CAPABILITY MAY BE SIMULATED.**

If you cannot verify a capability works on a real device, mark it `UNVERIFIED`. Do NOT mark it `IMPLEMENTED` because the code compiles.

Forbidden simulations:
- Fake root detection
- Fake network scanning / ARP / nmap / packet capture
- Fake process listings
- Fake filesystem contents
- Fake FPS counter
- Fake CPU/RAM/battery metrics
- Fake shell command output
- Fake PTY behavior

If a capability does not exist in code, it is `NOT_IMPLEMENTED`. If it exists in code but was not verified on a real device, it is `UNVERIFIED` (and `PARTIAL` in the runtime `CapabilityManager`).

---

## ⚠️ "DOCUMENTATION IS NOT PROOF OF IMPLEMENTATION"

The previous agent produced extensive documentation. That documentation describes intent, not verified behavior. You MUST verify each claim independently:

- Does the file exist? (`ls`)
- Does the code do what the doc says? (`cat` + read)
- Does it compile? (run the build)
- Do tests pass? (run the tests)
- Does it work on a real device? (install + test)

Only after all five questions are answered YES may a capability be marked `IMPLEMENTED` in your updated `PROJECT_STATUS.md`.

---

## Where the project should continue

The next priority (per the previous user conversation) is:

**Fix BUG #1 — Multi-line PASTE.**

But before writing any fix, the receiving agent MUST:

1. Reproduce the bug on a real Android device with the SAME IME the user is using.
2. Capture `adb logcat` output during the paste to see what text is actually being inserted into the EditText.
3. Determine the root cause (IME strips `\n` vs. EditText strips `\n` vs. something else).
4. Propose the fix (do NOT implement yet — wait for user confirmation).
5. After user approval, implement the fix.
6. Re-verify on the device.

**Do NOT continue with new features** (Vulkan, Lua, materials, GLB loader, picking, world map, visual effects) until:
- BUG #1 is resolved AND
- The terminal works correctly end-to-end on a real device AND
- The user explicitly authorizes new feature work.

---

## Critical files to know

| File | Purpose |
|---|---|
| `MASTER_SPEC.md` | What the project MUST be |
| `PROJECT_STATUS.md` | What the project IS right now |
| `MANIFEST_SHA256.txt` | Integrity check |
| `release/CyberEngine-current.apk` | The current shipped artifact |
| `release/busybox-arm64` | The bundled userland binary |
| `android/local.properties` | **MACHINE-SPECIFIC** — update with your SDK/NDK paths |
| `userland/build_busybox.sh` | Rebuilds the busybox binary from source |

---

## What NOT to do

- DO NOT mark anything as `IMPLEMENTED` without device verification.
- DO NOT add new features until BUG #1 is fixed.
- DO NOT modify `linux_bridge.cpp`, `busybox`, the PTY architecture, or the termios config without explicit user authorization.
- DO NOT clean up code "for aesthetics".
- DO NOT delete files.
- DO NOT reduce documentation.
- DO NOT hide bugs.
- DO NOT declare the project "done".
- DO NOT use the previously-leaked GitHub PAT (it has been or will be revoked by the time you read this).
