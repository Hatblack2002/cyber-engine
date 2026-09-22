// Cyber Engine :: Linux bridge (PTY-backed bundled busybox, no Termux)
//
// Architecture:
//   Kotlin → JNI → C++ Linux Bridge → PTY → libbusybox.so (executable
//   inside the apk_data_file-labeled native lib dir) → ash
//
// PENDING VERIFICATION: until a real `echo hello` runs on a real device,
// Capability::Terminal stays PARTIAL per spec §0.
//
// Comprehensive pre-flight + post-fork diagnostics: every step (posix_openpt,
// grantpt, unlockpt, fork, setsid, TIOCSCTTY, dup2[0/1/2], execve) is logged
// via stderr (which is the PTY slave — output appears in TerminalView).
// execve failures include errno + strerror so the cause is never hidden.
#include "engine/diagnostics/BootDiagnostics.hpp"

#include <jni.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <poll.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "CyberLinux", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "CyberLinux", __VA_ARGS__)

using cyber::BootDiagnostics;
using cyber::BootStage;

namespace {

struct PtySession {
    int masterFd = -1;
    pid_t childPid = -1;
    int lastExitStatus = -1;
};

constexpr size_t kMaxSessions = 8;
PtySession g_sessions[kMaxSessions];

int findFreeSession() {
    for (size_t i = 0; i < kMaxSessions; ++i) {
        if (g_sessions[i].masterFd == -1) return (int)i;
    }
    return -1;
}

// Emit a line to fd (typically 2 = stderr = PTY slave).
// Used by the child to surface diagnostics that the parent UI will read.
static void emitLine(int fd, const char* s) {
    if (fd >= 0) {
        ssize_t n = write(fd, s, strlen(s));
        (void)n;
    }
}

static void emitf(int fd, const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n > 0) emitLine(fd, buf);
}

// Spawn child in PTY. Each step logged.
// Returns 0 on success, -1 on error (with BootDiagnostics mark).
int spawnPty(const char* busyboxPath, const char* prefix, PtySession& out) {
    // -------- posix_openpt --------
    int masterFd = posix_openpt(O_RDWR | O_NOCTTY);
    if (masterFd < 0) {
        int err = errno;
        emitf(2, "[PTY][STEP_FAIL] posix_openpt errno=%d msg=%s\n", err, strerror(err));
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            std::string("posix_openpt errno=") + std::to_string(err));
        return -1;
    }
    LOGI("[PTY] posix_openpt OK masterFd=%d", masterFd);

    // -------- grantpt --------
    if (grantpt(masterFd) < 0) {
        int err = errno;
        emitf(2, "[PTY][STEP_FAIL] grantpt errno=%d msg=%s\n", err, strerror(err));
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            std::string("grantpt errno=") + std::to_string(err));
        close(masterFd);
        return -1;
    }
    LOGI("[PTY] grantpt OK");

    // -------- unlockpt --------
    if (unlockpt(masterFd) < 0) {
        int err = errno;
        emitf(2, "[PTY][STEP_FAIL] unlockpt errno=%d msg=%s\n", err, strerror(err));
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            std::string("unlockpt errno=") + std::to_string(err));
        close(masterFd);
        return -1;
    }
    LOGI("[PTY] unlockpt OK");

    // -------- ptsname --------
    const char* slaveName = ptsname(masterFd);
    if (!slaveName) {
        int err = errno;
        emitf(2, "[PTY][STEP_FAIL] ptsname errno=%d msg=%s\n", err, strerror(err));
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            std::string("ptsname errno=") + std::to_string(err));
        close(masterFd);
        return -1;
    }
    LOGI("[PTY] ptsname OK slave=%s", slaveName);

    // -------- open slave --------
    int slaveFd = open(slaveName, O_RDWR);
    if (slaveFd < 0) {
        int err = errno;
        emitf(2, "[PTY][STEP_FAIL] open(slave) errno=%d msg=%s\n", err, strerror(err));
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            std::string("open(slave) errno=") + std::to_string(err));
        close(masterFd);
        return -1;
    }
    LOGI("[PTY] open(slave) OK slaveFd=%d", slaveFd);

    // Set window size so apps using TIOCGWINSZ don't get garbage.
    struct winsize ws;
    ws.ws_row = 24; ws.ws_col = 80; ws.ws_xpixel = 0; ws.ws_ypixel = 0;
    ioctl(slaveFd, TIOCSWINSZ, &ws);

    // -------- fork --------
    pid_t pid = fork();
    if (pid < 0) {
        int err = errno;
        emitf(2, "[PTY][STEP_FAIL] fork errno=%d msg=%s\n", err, strerror(err));
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            std::string("fork errno=") + std::to_string(err));
        close(slaveFd); close(masterFd);
        return -1;
    }

    if (pid == 0) {
        // ===== CHILD =====
        close(masterFd);

        // -------- setsid --------
        pid_t sid = setsid();
        if (sid < 0) {
            int err = errno;
            emitf(2, "[PTY][STEP_FAIL][CHILD] setsid errno=%d msg=%s\n", err, strerror(err));
            _exit(126);
        }
        emitf(2, "[PTY][STEP_OK][CHILD] setsid sid=%d\n", (int)sid);

        // -------- TIOCSCTTY (set slave as controlling terminal) --------
        if (ioctl(slaveFd, TIOCSCTTY, 0) < 0) {
            int err = errno;
            emitf(2, "[PTY][STEP_FAIL][CHILD] TIOCSCTTY errno=%d msg=%s\n", err, strerror(err));
            // Not fatal — ash doesn't strictly require a controlling tty.
        } else {
            emitf(2, "[PTY][STEP_OK][CHILD] TIOCSCTTY\n");
        }

        // -------- dup2 stdin/stdout/stderr --------
        if (dup2(slaveFd, 0) < 0) {
            int err = errno;
            emitf(2, "[PTY][STEP_FAIL][CHILD] dup2(stdin) errno=%d msg=%s\n", err, strerror(err));
            _exit(126);
        }
        emitf(2, "[PTY][STEP_OK][CHILD] dup2(stdin)\n");

        if (dup2(slaveFd, 1) < 0) {
            int err = errno;
            emitf(2, "[PTY][STEP_FAIL][CHILD] dup2(stdout) errno=%d msg=%s\n", err, strerror(err));
            _exit(126);
        }
        emitf(2, "[PTY][STEP_OK][CHILD] dup2(stdout)\n");

        if (dup2(slaveFd, 2) < 0) {
            int err = errno;
            emitf(2, "[PTY][STEP_FAIL][CHILD] dup2(stderr) errno=%d msg=%s\n", err, strerror(err));
            _exit(126);
        }
        emitf(2, "[PTY][STEP_OK][CHILD] dup2(stderr)\n");

        if (slaveFd > 2) close(slaveFd);

        // -------- Disable PTY ECHO (spec §0 — manual echo in UI) --------
        // We control the visual echo ourselves in TerminalView so the input
        // is displayed exactly once. With ECHO off, typed chars go to the
        // shell's line buffer without being echoed back.
        struct termios tio;
        if (tcgetattr(0, &tio) == 0) {
            tio.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL);
            // Keep ICANON (line-buffered) and ISIG (Ctrl+C → SIGINT, Ctrl+D → EOF).
            tio.c_lflag |= (ICANON | ISIG);
            // Map CR → NL so the shell sees \n on Enter (\r sent).
            tio.c_iflag |= ICRNL;
            tcsetattr(0, TCSANOW, &tio);
            emitf(2, "[PTY][STEP_OK][CHILD] termios ECHO=off ICANON=on ISIG=on\n");
        } else {
            int err = errno;
            emitf(2, "[PTY][STEP_WARN][CHILD] tcgetattr errno=%d msg=%s\n", err, strerror(err));
        }

        // -------- Pre-execve stat() the executable --------
        struct stat st;
        int statRc = stat(busyboxPath, &st);
        if (statRc != 0) {
            int err = errno;
            emitf(2, "[PTY][PRE_EXEC_STAT_FAIL] path=%s errno=%d msg=%s\n",
                  busyboxPath, err, strerror(err));
            _exit(126);
        }
        emitf(2, "[PTY][PRE_EXEC] path=%s mode=0%o size=%ld exec_bit=%d\n",
              busyboxPath, st.st_mode & 07777, (long)st.st_size,
              (st.st_mode & S_IXUSR) ? 1 : 0);

        // -------- Build environment --------
        std::string pathEnv = std::string("PATH=") + prefix + "/bin:" + prefix + "/sbin:" + prefix + "/usr/bin:/system/bin:/system/xbin";
        std::string homeEnv = std::string("HOME=") + prefix + "/home";
        std::string tmpEnv  = std::string("TMPDIR=") + prefix + "/tmp";
        std::string termEnv  = "TERM=xterm-256color";
        std::string langEnv  = "LANG=C.UTF-8";
        std::string lcEnv     = "LC_ALL=C.UTF-8";
        std::string ceEnv     = "CYBER_ENGINE=1";
        std::string ceuEnv    = std::string("CYBER_USERLAND=") + prefix;
        std::string ps1Env    = "PS1=cyber# ";

        emitf(2, "[PTY][ENV] %s\n", pathEnv.c_str());
        emitf(2, "[PTY][ENV] %s\n", homeEnv.c_str());
        emitf(2, "[PTY][ENV] %s\n", tmpEnv.c_str());
        emitf(2, "[PTY][ENV] %s\n", termEnv.c_str());
        emitf(2, "[PTY][ENV] CYBER_ENGINE=1\n");
        emitf(2, "[PTY][ENV] %s\n", ceuEnv.c_str());

        std::vector<std::string> envStrings = {
            pathEnv, homeEnv, tmpEnv, termEnv, langEnv, lcEnv, ceEnv, ceuEnv, ps1Env
        };
        std::vector<char*> envp;
        envp.reserve(envStrings.size() + 1);
        for (auto& s : envStrings) envp.push_back(&s[0]);
        envp.push_back(nullptr);

        char* argv[] = {
            const_cast<char*>("busybox"),
            const_cast<char*>("ash"),
            const_cast<char*>("--login"),
            nullptr
        };

        // -------- chdir to HOME --------
        std::string homeDir = std::string(prefix) + "/home";
        if (chdir(homeDir.c_str()) != 0) {
            int err = errno;
            emitf(2, "[PTY][STEP_WARN][CHILD] chdir(%s) errno=%d msg=%s\n",
                  homeDir.c_str(), err, strerror(err));
        } else {
            emitf(2, "[PTY][STEP_OK][CHILD] chdir(%s)\n", homeDir.c_str());
        }

        // -------- execve --------
        emitf(2, "[PTY][EXEC_ATTEMPT] path=%s argv0=busybox argv1=ash argv2=--login\n",
              busyboxPath);
        execve(busyboxPath, argv, envp.data());

        // If we reach here, execve FAILED. Capture errno IMMEDIATELY.
        int err = errno;
        emitf(2, "[PTY][EXEC_ERROR] path=%s errno=%d message=%s\n",
              busyboxPath, err, strerror(err));
        _exit(127);
    }

    // ===== PARENT =====
    close(slaveFd);
    out.masterFd = masterFd;
    out.childPid = pid;
    out.lastExitStatus = -1;
    return 0;
}

bool reapIfDead(PtySession& s) {
    if (s.childPid <= 0) return false;
    int status = 0;
    pid_t r = waitpid(s.childPid, &status, WNOHANG);
    if (r == s.childPid) {
        s.lastExitStatus = status;
        s.childPid = -1;
        return true;
    }
    return false;
}

} // namespace

// ===== JNI entry points =====

extern "C" {

// Returns session id >= 0, or -1 on error.
JNIEXPORT jint JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalStart(JNIEnv* env, jclass, jstring jBusyboxPath, jstring jPrefix) {
    const char* busyboxPath = env->GetStringUTFChars(jBusyboxPath, nullptr);
    const char* prefix = env->GetStringUTFChars(jPrefix, nullptr);

    // ===== PRE-FLIGHT (parent side, before fork) =====
    LOGI("[PTY][PREFLIGHT] busyboxPath=%s prefix=%s", busyboxPath, prefix);

    struct stat st;
    int statRc = stat(busyboxPath, &st);
    if (statRc != 0) {
        int err = errno;
        LOGE("[PTY][PREFLIGHT] stat FAILED errno=%d msg=%s path=%s", err, strerror(err), busyboxPath);
        BootDiagnostics::instance().fail(BootStage::BusyBoxLocated,
            std::string("stat errno=") + std::to_string(err) + " path=" + busyboxPath);
        env->ReleaseStringUTFChars(jBusyboxPath, busyboxPath);
        env->ReleaseStringUTFChars(jPrefix, prefix);
        return -1;
    }
    LOGI("[PTY][PREFLIGHT] stat OK mode=0%o size=%ld exec_bit=%d",
         st.st_mode & 07777, (long)st.st_size, (st.st_mode & S_IXUSR) ? 1 : 0);
    BootDiagnostics::instance().ok(BootStage::BusyBoxLocated,
        std::string("path=") + busyboxPath + " size=" + std::to_string((long)st.st_size));

    if (!(st.st_mode & S_IXUSR)) {
        LOGE("[PTY][PREFLIGHT] EXEC BIT NOT SET on %s", busyboxPath);
        BootDiagnostics::instance().fail(BootStage::BusyBoxLocated,
            "exec bit not set on busybox");
        env->ReleaseStringUTFChars(jBusyboxPath, busyboxPath);
        env->ReleaseStringUTFChars(jPrefix, prefix);
        return -1;
    }

    int id = findFreeSession();
    if (id < 0) {
        BootDiagnostics::instance().fail(BootStage::PTYInitialized, "no free session slot");
        env->ReleaseStringUTFChars(jBusyboxPath, busyboxPath);
        env->ReleaseStringUTFChars(jPrefix, prefix);
        return -1;
    }
    PtySession& s = g_sessions[id];
    int rc = spawnPty(busyboxPath, prefix, s);

    env->ReleaseStringUTFChars(jBusyboxPath, busyboxPath);
    env->ReleaseStringUTFChars(jPrefix, prefix);

    if (rc != 0) {
        BootDiagnostics::instance().fail(BootStage::PTYInitialized,
            "spawnPty returned error — see logcat CyberLinux");
        return -1;
    }
    BootDiagnostics::instance().ok(BootStage::PTYInitialized,
        "posix_openpt + grantpt + unlockpt + fork + setsid + dup2 completed");
    BootDiagnostics::instance().ok(BootStage::ShellSessionStarted,
        "ash child spawned — pending execve verification on device");
    LOGI("[PTY] session %d started: pid=%d masterFd=%d", id, s.childPid, s.masterFd);
    return id;
}

JNIEXPORT jint JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalRead(JNIEnv* env, jclass, jint sid, jbyteArray buf) {
    if (sid < 0 || sid >= (jint)kMaxSessions) return -1;
    PtySession& s = g_sessions[sid];
    if (s.masterFd < 0) return -1;
    jsize cap = env->GetArrayLength(buf);
    if (cap <= 0) return -1;
    std::vector<uint8_t> tmp(cap);
    ssize_t n = read(s.masterFd, tmp.data(), cap);
    if (n <= 0) {
        if (reapIfDead(s)) {
            LOGI("[PTY] child exited status=%d (exit_code=%d signal=%d)",
                 s.lastExitStatus,
                 (s.lastExitStatus >> 8) & 0xff,
                 s.lastExitStatus & 0xff);
        }
        return -1;
    }
    env->SetByteArrayRegion(buf, 0, (jsize)n, reinterpret_cast<jbyte*>(tmp.data()));
    return (jint)n;
}

JNIEXPORT jint JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalWrite(JNIEnv* env, jclass, jint sid, jbyteArray buf, jint len) {
    if (sid < 0 || sid >= (jint)kMaxSessions) return -1;
    PtySession& s = g_sessions[sid];
    if (s.masterFd < 0) return -1;
    jbyte* data = env->GetByteArrayElements(buf, nullptr);
    ssize_t n = write(s.masterFd, data, len);
    env->ReleaseByteArrayElements(buf, data, JNI_ABORT);
    return (jint)n;
}

JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalSignal(JNIEnv*, jclass, jint sid, jint signal) {
    if (sid < 0 || sid >= (jint)kMaxSessions) return;
    PtySession& s = g_sessions[sid];
    if (s.childPid > 0) {
        kill(s.childPid, signal);
    }
}

JNIEXPORT jint JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalExitStatus(JNIEnv*, jclass, jint sid) {
    if (sid < 0 || sid >= (jint)kMaxSessions) return -1;
    PtySession& s = g_sessions[sid];
    reapIfDead(s);
    return s.lastExitStatus;
}

JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalStop(JNIEnv*, jclass, jint sid) {
    if (sid < 0 || sid >= (jint)kMaxSessions) return;
    PtySession& s = g_sessions[sid];
    if (s.masterFd >= 0) {
        close(s.masterFd);
        s.masterFd = -1;
    }
    if (s.childPid > 0) {
        int status = 0;
        kill(s.childPid, SIGTERM);
        for (int i = 0; i < 20; ++i) {
            if (waitpid(s.childPid, &status, WNOHANG) == s.childPid) {
                s.lastExitStatus = status;
                s.childPid = -1;
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        kill(s.childPid, SIGKILL);
        waitpid(s.childPid, &status, 0);
        s.lastExitStatus = status;
        s.childPid = -1;
    }
}

JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeTerminalResize(JNIEnv*, jclass, jint sid, jint rows, jint cols) {
    if (sid < 0 || sid >= (jint)kMaxSessions) return;
    PtySession& s = g_sessions[sid];
    if (s.masterFd < 0) return;
    struct winsize ws;
    ws.ws_row = (unsigned short)rows;
    ws.ws_col = (unsigned short)cols;
    ws.ws_xpixel = 0; ws.ws_ypixel = 0;
    ioctl(s.masterFd, TIOCSWINSZ, &ws);
    if (s.childPid > 0) kill(s.childPid, SIGWINCH);
}

} // extern "C"
