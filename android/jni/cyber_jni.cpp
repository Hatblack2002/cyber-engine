// Cyber Engine :: JNI bridge implementation
// Exposes engine lifecycle, shell, capabilities, renderer, scene orbit,
// PTY-backed terminal, and BOOT diagnostics to Kotlin.
// Every boot stage is recorded in BootDiagnostics so the UI can show exactly
// where the boot stopped if it fails.
#include "cyber_jni.hpp"
#include "jni_state.hpp"

#include "engine/Engine.hpp"
#include "engine/shell/Shell.hpp"
#include "engine/capabilities/CapabilityManager.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include "engine/diagnostics/BootDiagnostics.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/renderer/IRenderer.hpp"
#include "engine/scene/Scene.hpp"
#include "engine/memory/SystemAllocator.hpp"

#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <sstream>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace cyber;

namespace {
Shell* g_shell = nullptr;
ShellContext makeShellContext() {
    ShellContext ctx;
    ctx.getVersion = []() -> std::string {
        std::string out;
        Shell s(ShellContext{});
        s.execute("version", out);
        return out;
    };
    ctx.getCapabilities = []() -> std::string {
        std::ostringstream os;
        for (const auto& c : CapabilityManager::instance().all()) {
            os << capabilityName(c.cap) << "|" << capabilityStateLabel(c.state)
               << "|" << (c.note.empty() ? "-" : c.note) << "\n";
        }
        return os.str();
    };
    ctx.getDiagnostics = []() -> std::string {
        std::ostringstream os;
        std::array<DiagEntry, Diagnostics::kCapacity> buf{};
        Span<DiagEntry> view(buf.data(), buf.size());
        auto snap = Diagnostics::instance().snapshot(view);
        for (size_t i = 0; i < snap.size(); ++i) {
            const auto& e = snap[i];
            const char* lvl = "?";
            switch (e.level) {
                case DiagLevel::Info:  lvl = "INFO"; break;
                case DiagLevel::Warn:  lvl = "WARN"; break;
                case DiagLevel::Error: lvl = "ERROR"; break;
                case DiagLevel::Fatal: lvl = "FATAL"; break;
            }
            os << "[" << e.monotonicMs << "] " << lvl << " " << e.tag << ": " << e.message << "\n";
        }
        return os.str();
    };
    return ctx;
}
} // namespace

// ===== Engine lifecycle =====

extern "C" JNIEXPORT jlong JNICALL
Java_com_cyberengine_app_CyberBridge_nativeEngineCreate(JNIEnv*, jclass) {
    LOGI("nativeEngineCreate");
    BootDiagnostics::instance().ok(BootStage::JNILibraryLoaded, "libcyberengine_jni.so loaded");

    auto& e = Engine::instance();
    auto r = e.create(nullptr, nullptr);
    if (!r.ok()) {
        BootDiagnostics::instance().fail(BootStage::EngineCreated, "Engine::create returned non-Ok");
        LOGE("Engine create failed");
        return 0;
    }
    BootDiagnostics::instance().ok(BootStage::EngineCreated, "Engine booted headless");

    // Real renderer (GLES only on Android; Vulkan NOT_IMPLEMENTED per spec §5).
    BootDiagnostics::instance().ok(BootStage::RendererCreationStarted, "createRenderer() invoked");
    IRenderer* renderer = createRenderer();
    cyber_jni::rendererSlot() = renderer;
    if (renderer) {
        BootDiagnostics::instance().ok(BootStage::EGLInitialization,
            "EGL display+context created (validated in createRenderer path)");
        BootDiagnostics::instance().ok(BootStage::GLESContextCreated,
            "GLES3 context — final validation deferred to onSurfaceCreated");
        LOGI("GLES renderer created: %s", renderer->backendName());
    } else {
        BootDiagnostics::instance().fail(BootStage::RendererCreationStarted,
            "createRenderer() returned nullptr");
    }

    // Real scene (singleton in JNI layer)
    static Scene scene;
    scene.initialize();
    cyber_jni::sceneSlot() = &scene;

    // Shell
    if (!g_shell) g_shell = new Shell(makeShellContext());

    // Register the capabilities that were missing before (spec §6).
    CapabilityManager::instance().registerCapability(
        Capability::AppPrivateStorage, CapabilityState::Available,
        "ctx.filesDir — always available without permission");
    CapabilityManager::instance().registerCapability(
        Capability::SharedStorage, CapabilityState::RequiresPermission,
        "READ_EXTERNAL_STORAGE on API<30; Scoped Storage on API30+");
    CapabilityManager::instance().registerCapability(
        Capability::SAF, CapabilityState::RequiresPermission,
        "Storage Access Framework — needs user grant; not yet wired");
    CapabilityManager::instance().registerCapability(
        Capability::Filesystem, CapabilityState::Partial,
        "App-private storage only in P01; SAF pending");
    CapabilityManager::instance().registerCapability(
        Capability::SystemInfo, CapabilityState::Available,
        "RAM, battery, CPU count, storage via StatFs");
    CapabilityManager::instance().setState(
        Capability::Terminal, CapabilityState::Partial,
        "PTY + bundled busybox — not yet verified on real device");
    CapabilityManager::instance().setState(
        Capability::Shell, CapabilityState::Available,
        "Cyber shell built-ins + bundled userland");

    BootDiagnostics::instance().ok(BootStage::LinuxBridgeInitialized,
        "linux_bridge.cpp compiled in (fork/setsid/dup2/posix_openpt)");
    // Note: PTYInitialized is marked from nativeTerminalStart when a session is spawned.

    return cyber_jni::handleFromEngine(&e);
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeEngineDestroy(JNIEnv*, jclass, jlong) {
    LOGI("nativeEngineDestroy");
    Engine::instance().destroy();
    if (g_shell) { delete g_shell; g_shell = nullptr; }
    cyber_jni::rendererSlot() = nullptr;
    cyber_jni::sceneSlot()    = nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeOnPause(JNIEnv*, jclass, jlong) {
    Engine::instance().onPause();
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeOnResume(JNIEnv*, jclass, jlong) {
    Engine::instance().onResume();
}

// ===== Surface (GLES) =====

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeOnSurfaceCreated(JNIEnv* env, jclass, jlong, jobject surface) {
    ANativeWindow* win = ANativeWindow_fromSurface(env, surface);
    LOGI("nativeOnSurfaceCreated win=%p", win);
    IRenderer* r = cyber_jni_getRenderer();
    if (!r) {
        BootDiagnostics::instance().fail(BootStage::SurfaceCreated, "renderer slot is null");
        return;
    }
    auto s = r->onSurfaceCreated(win);
    if (s == Status::Ok) {
        BootDiagnostics::instance().ok(BootStage::SurfaceCreated, "EGL window surface + GLES current");
    } else {
        BootDiagnostics::instance().fail(BootStage::SurfaceCreated,
            std::string("onSurfaceCreated returned ") + statusLabel(s));
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeOnSurfaceDestroyed(JNIEnv*, jclass, jlong) {
    IRenderer* r = cyber_jni_getRenderer();
    if (r) r->onSurfaceDestroyed();
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeOnSurfaceResized(JNIEnv*, jclass, jlong, jint w, jint h) {
    IRenderer* r = cyber_jni_getRenderer();
    if (r) r->onSurfaceResized(w, h);
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeRender(JNIEnv*, jclass, jlong) {
    IRenderer* r = cyber_jni_getRenderer();
    if (!r) return;
    Engine::instance().tick();
    Scene* s = cyber_jni_getScene();
    r->render(s);
}

// ===== Touch =====

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeOrbitCamera(JNIEnv*, jclass, jlong, jfloat dYaw, jfloat dPitch) {
    Scene* s = cyber_jni_getScene();
    if (s) s->orbitCamera(dYaw, dPitch);
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeZoomCamera(JNIEnv*, jclass, jlong, jfloat factor) {
    Scene* s = cyber_jni_getScene();
    if (s) s->zoomCamera(factor);
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativePanCamera(JNIEnv*, jclass, jlong, jfloat dx, jfloat dy) {
    Scene* s = cyber_jni_getScene();
    if (s) s->panCamera(dx, dy);
}

// ===== Shell =====

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyberengine_app_CyberBridge_nativeShellExecute(JNIEnv* env, jclass, jlong, jstring cmd) {
    const char* s = env->GetStringUTFChars(cmd, nullptr);
    std::string out;
    if (g_shell) g_shell->execute(s, out);
    env->ReleaseStringUTFChars(cmd, s);
    return env->NewStringUTF(out.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyberengine_app_CyberBridge_nativeCapabilities(JNIEnv* env, jclass, jlong) {
    std::ostringstream os;
    for (const auto& c : CapabilityManager::instance().all()) {
        os << capabilityName(c.cap) << "|" << capabilityStateLabel(c.state)
           << "|" << (c.note.empty() ? "-" : c.note) << "\n";
    }
    return env->NewStringUTF(os.str().c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyberengine_app_CyberBridge_nativeDiagnostics(JNIEnv* env, jclass, jlong) {
    std::ostringstream os;
    std::array<DiagEntry, Diagnostics::kCapacity> buf{};
    Span<DiagEntry> view(buf.data(), buf.size());
    auto snap = Diagnostics::instance().snapshot(view);
    for (size_t i = 0; i < snap.size(); ++i) {
        const auto& e = snap[i];
        const char* lvl = "INFO";
        switch (e.level) {
            case DiagLevel::Warn:  lvl = "WARN"; break;
            case DiagLevel::Error: lvl = "ERROR"; break;
            case DiagLevel::Fatal: lvl = "FATAL"; break;
            default: break;
        }
        os << "[" << e.monotonicMs << "] " << lvl << " " << e.tag << ": " << e.message << "\n";
    }
    return env->NewStringUTF(os.str().c_str());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_cyberengine_app_CyberBridge_nativeRendererFrameCount(JNIEnv*, jclass, jlong) {
    IRenderer* r = cyber_jni_getRenderer();
    return r ? r->frameCount() : 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyberengine_app_CyberBridge_nativeRendererBackend(JNIEnv* env, jclass, jlong) {
    IRenderer* r = cyber_jni_getRenderer();
    return env->NewStringUTF(r ? r->backendName() : "none");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_cyberengine_app_CyberBridge_nativeBootLog(JNIEnv* env, jclass, jlong) {
    return env->NewStringUTF(BootDiagnostics::instance().dump().c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_cyberengine_app_CyberBridge_nativeBootMark(JNIEnv* env, jclass, jlong, jint stage, jboolean ok, jstring jreason) {
    std::string reason;
    if (jreason) {
        const char* r = env->GetStringUTFChars(jreason, nullptr);
        if (r) { reason = r; env->ReleaseStringUTFChars(jreason, r); }
    }
    BootDiagnostics::instance().mark(static_cast<BootStage>(stage), ok != JNI_FALSE, reason);
}
