// cyber-engine :: GLES renderer (spec §5 — EGL clear + swap + resize + real mesh shader)
#pragma once
#include "engine/renderer/IRenderer.hpp"
#include "engine/capabilities/CapabilityManager.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>

#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
struct ANativeWindow;
#else
// Host build: this header should NOT be compiled. If it is, fail loudly.
#  error "GLESRenderer.hpp must only be built for Android."
#endif

namespace cyber {

class Scene;
class Mesh;

// Real GLES3 backend backed by EGL.
// - Vulkan is NOT_IMPLEMENTED in P01 per spec §5.
// - GLES clear+swap is DONE; mesh shader is real (cube + perspective + camera).
// - Frame counter is incremented only after a successful swap.
class GLESRenderer final : public IRenderer {
public:
    GLESRenderer() = default;
    ~GLESRenderer() override { shutdown(); }

    Status initialize() noexcept override;
    Status shutdown() noexcept override;
    Status onPause() noexcept override;
    Status onResume() noexcept override;
    Status onSurfaceCreated(void* nativeWindow) noexcept override;
    Status onSurfaceDestroyed() noexcept override;
    Status onSurfaceResized(int32_t w, int32_t h) noexcept override;
    Result<void> render(Scene* scene) noexcept override;

    const char* backendName() const noexcept override { return "GLES3 (EGL)"; }
    int64_t frameCount() const noexcept override { return frames_; }
    double lastFrameMs() const noexcept override { return lastFrameMs_; }

private:
    Status initEglContext() noexcept;
    Status destroyEglSurface() noexcept;
    Status makeCurrent() noexcept;

    // Simple shader program (vertex + fragment) for the cube mesh.
    Status buildProgram() noexcept;

    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLConfig  config_  = nullptr;
    EGLContext context_ = EGL_NO_CONTEXT;
    EGLSurface surface_ = EGL_NO_SURFACE;
    ANativeWindow* window_ = nullptr;

    int32_t width_  = 0;
    int32_t height_ = 0;
    bool     paused_ = false;
    std::atomic<int64_t> frames_{0};
    double lastFrameMs_ = 0.0;

    // GL program handles (GLuint)
    uint32_t program_ = 0;
    uint32_t vboCube_ = 0;
    uint32_t iboCube_ = 0;
    int32_t  aPos_ = -1;
    int32_t  aColor_ = -1;
    int32_t  uMVP_ = -1;
    int32_t  uLight_ = -1;
};

} // namespace cyber
