// cyber-engine :: GLES renderer implementation
// Real EGL + GLES3. Compiles only on Android (NDK provides EGL/GLES3 headers).
#if !defined(__ANDROID__)
#  error "GLESRenderer.cpp must be built for Android (NDK). Host build should skip this file."
#endif

#include "engine/renderer/gles/GLESRenderer.hpp"
#include "engine/scene/Scene.hpp"
#include "engine/capabilities/CapabilityManager.hpp"
#include "engine/diagnostics/Diagnostics.hpp"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>

#include <cmath>
#include <cstring>
#include <chrono>
#include <vector>

namespace cyber {

// Simple cube: 24 verts (4 per face) so each face has its own normal/color.
// We use per-vertex color instead of normals for the cyber look.
static const float kCubeVerts[] = {
    // pos (x,y,z)        color (r,g,b)
    // -X face
    -1.0f,-1.0f,-1.0f,    0.00f, 0.94f, 1.00f,
    -1.0f, 1.0f,-1.0f,    0.00f, 0.94f, 1.00f,
    -1.0f, 1.0f, 1.0f,    0.00f, 0.94f, 1.00f,
    -1.0f,-1.0f, 1.0f,    0.00f, 0.94f, 1.00f,
    // +X face
     1.0f,-1.0f, 1.0f,    0.00f, 1.00f, 0.25f,
     1.0f, 1.0f, 1.0f,    0.00f, 1.00f, 0.25f,
     1.0f, 1.0f,-1.0f,    0.00f, 1.00f, 0.25f,
     1.0f,-1.0f,-1.0f,    0.00f, 1.00f, 0.25f,
    // -Y face
    -1.0f,-1.0f,-1.0f,    0.13f, 0.40f, 0.70f,
     1.0f,-1.0f,-1.0f,    0.13f, 0.40f, 0.70f,
     1.0f,-1.0f, 1.0f,    0.13f, 0.40f, 0.70f,
    -1.0f,-1.0f, 1.0f,    0.13f, 0.40f, 0.70f,
    // +Y face
    -1.0f, 1.0f, 1.0f,    0.70f, 0.13f, 0.40f,
     1.0f, 1.0f, 1.0f,    0.70f, 0.13f, 0.40f,
     1.0f, 1.0f,-1.0f,    0.70f, 0.13f, 0.40f,
    -1.0f, 1.0f,-1.0f,    0.70f, 0.13f, 0.40f,
    // -Z face
    -1.0f,-1.0f,-1.0f,    0.10f, 0.10f, 0.30f,
     1.0f,-1.0f,-1.0f,    0.10f, 0.10f, 0.30f,
     1.0f, 1.0f,-1.0f,    0.10f, 0.10f, 0.30f,
    -1.0f, 1.0f,-1.0f,    0.10f, 0.10f, 0.30f,
    // +Z face
    -1.0f,-1.0f, 1.0f,    0.20f, 0.80f, 0.80f,
     1.0f,-1.0f, 1.0f,    0.20f, 0.80f, 0.80f,
     1.0f, 1.0f, 1.0f,    0.20f, 0.80f, 0.80f,
    -1.0f, 1.0f, 1.0f,    0.20f, 0.80f, 0.80f,
};
static const uint16_t kCubeIndices[] = {
     0, 1, 2,  0, 2, 3,    // -X
     4, 5, 6,  4, 6, 7,    // +X
     8, 9,10,  8,10,11,    // -Y
    12,13,14, 12,14,15,    // +Y
    16,17,18, 16,18,19,    // -Z
    20,21,22, 20,22,23,    // +Z
};

static const char* kVS = R"GLSL(#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
uniform mat4 uMVP;
out vec3 vColor;
void main(){
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)GLSL";
static const char* kFS = R"GLSL(#version 300 es
precision mediump float;
in vec3 vColor;
uniform vec3 uLight; // direction
out vec4 frag;
void main(){
    float shade = 0.5 + 0.5 * abs(dot(normalize(vColor), normalize(uLight)));
    frag = vec4(vColor * shade, 1.0);
}
)GLSL";

static GLuint compileShader(GLenum type, const char* src) noexcept {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        CYBER_LOG_ERROR("gles", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

Status GLESRenderer::initialize() noexcept {
    CapabilityManager::instance().setState(
        Capability::Renderer_GLES, CapabilityState::Available,
        "EGL/GLES3 backend available");
    return Status::Ok;
}

Status GLESRenderer::buildProgram() noexcept {
    if (program_) return Status::Ok;
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFS);
    if (!vs || !fs) return Status::Error;
    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint linkOk = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &linkOk);
    if (!linkOk) {
        char log[1024];
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        CYBER_LOG_ERROR("gles", log);
        return Status::Error;
    }
    aPos_   = glGetAttribLocation(program_, "aPos");
    aColor_ = glGetAttribLocation(program_, "aColor");
    uMVP_   = glGetUniformLocation(program_, "uMVP");
    uLight_ = glGetUniformLocation(program_, "uLight");

    glGenBuffers(1, &vboCube_);
    glBindBuffer(GL_ARRAY_BUFFER, vboCube_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);
    glGenBuffers(1, &iboCube_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCubeIndices), kCubeIndices, GL_STATIC_DRAW);

    return Status::Ok;
}

Status GLESRenderer::initEglContext() noexcept {
    if (display_ != EGL_NO_DISPLAY) return Status::Ok;
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY) return Status::Unavailable;
    EGLint major, minor;
    if (!eglInitialize(display_, &major, &minor)) return Status::Error;

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };
    EGLint n = 0;
    if (!eglChooseConfig(display_, configAttribs, &config_, 1, &n) || n < 1) {
        return Status::Unavailable;
    }
    EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, ctxAttribs);
    if (context_ == EGL_NO_CONTEXT) return Status::Error;

    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, context_);
    CapabilityManager::instance().setState(
        Capability::Renderer_GLES, CapabilityState::Available,
        "EGL context created");
    return Status::Ok;
}

Status GLESRenderer::destroyEglSurface() noexcept {
    if (surface_ != EGL_NO_SURFACE) {
        eglDestroySurface(display_, surface_);
        surface_ = EGL_NO_SURFACE;
    }
    return Status::Ok;
}

Status GLESRenderer::makeCurrent() noexcept {
    if (display_ == EGL_NO_DISPLAY || surface_ == EGL_NO_SURFACE) return Status::BadState;
    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        return Status::Error;
    }
    return Status::Ok;
}

Status GLESRenderer::onSurfaceCreated(void* nativeWindow) noexcept {
    window_ = static_cast<ANativeWindow*>(nativeWindow);
    auto s = initEglContext();
    if (s != Status::Ok) return s;

    destroyEglSurface();
    EGLint format = 0;
    eglGetConfigAttrib(display_, config_, EGL_NATIVE_VISUAL_ID, &format);
    if (window_) ANativeWindow_setBuffersGeometry(window_, 0, 0, format);

    surface_ = eglCreateWindowSurface(display_, config_, window_, nullptr);
    if (surface_ == EGL_NO_SURFACE) return Status::Error;
    if (auto m = makeCurrent(); m != Status::Ok) return m;

    int32_t w = ANativeWindow_getWidth(window_);
    int32_t h = ANativeWindow_getHeight(window_);
    width_ = w; height_ = h;
    glViewport(0, 0, w, h);

    auto bp = buildProgram();
    if (bp != Status::Ok) return bp;

    glClearColor(0.02f, 0.04f, 0.06f, 1.0f); // cyber dark
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return Status::Ok;
}

Status GLESRenderer::onSurfaceDestroyed() noexcept {
    destroyEglSurface();
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, context_);
    }
    if (window_) { ANativeWindow_release(window_); window_ = nullptr; }
    return Status::Ok;
}

Status GLESRenderer::onSurfaceResized(int32_t w, int32_t h) noexcept {
    width_ = w; height_ = h;
    if (auto m = makeCurrent(); m == Status::Ok) {
        glViewport(0, 0, w, h);
    }
    return Status::Ok;
}

Status GLESRenderer::onPause() noexcept {
    paused_ = true;
    return Status::Ok;
}

Status GLESRenderer::onResume() noexcept {
    paused_ = false;
    return Status::Ok;
}

Result<void> GLESRenderer::render(Scene* scene) noexcept {
    if (paused_ || surface_ == EGL_NO_SURFACE) return Result<void>::Err(Status::Unavailable);
    if (auto m = makeCurrent(); m != Status::Ok) return Result<void>::Err(m);

    auto t0 = std::chrono::steady_clock::now();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (program_) {
        glUseProgram(program_);

        // Build MVP from scene camera + cube model
        float mvp[16];
        if (scene) scene->computeMVPForCube(mvp, width_, height_);
        else {
            // Fallback identity-ish projection
            for (int i = 0; i < 16; ++i) mvp[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        }
        glUniformMatrix4fv(uMVP_, 1, GL_FALSE, mvp);
        glUniform3f(uLight_, 0.4f, 0.8f, 0.3f);

        glBindBuffer(GL_ARRAY_BUFFER, vboCube_);
        glEnableVertexAttribArray(aPos_);
        glVertexAttribPointer(aPos_, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(aColor_);
        glVertexAttribPointer(aColor_, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube_);
        glDrawElements(GL_TRIANGLES, sizeof(kCubeIndices) / sizeof(kCubeIndices[0]),
                       GL_UNSIGNED_SHORT, nullptr);
    }

    auto t1 = std::chrono::steady_clock::now();
    lastFrameMs_ = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!eglSwapBuffers(display_, surface_)) {
        EGLint err = eglGetError();
        if (err == EGL_BAD_SURFACE) {
            // Surface lost — UI must recreate. Engine remains alive.
            return Result<void>::Err(Status::BadState);
        }
        return Result<void>::Err(Status::Error);
    }
    frames_.fetch_add(1, std::memory_order_relaxed);
    return Result<void>::Ok();
}

Status GLESRenderer::shutdown() noexcept {
    if (program_) { glDeleteProgram(program_); program_ = 0; }
    if (vboCube_) { glDeleteBuffers(1, &vboCube_); vboCube_ = 0; }
    if (iboCube_) { glDeleteBuffers(1, &iboCube_); iboCube_ = 0; }
    destroyEglSurface();
    if (context_ != EGL_NO_CONTEXT) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(display_, context_);
        context_ = EGL_NO_CONTEXT;
    }
    if (display_ != EGL_NO_DISPLAY) {
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
    return Status::Ok;
}

// Factory: spec §5 order is Vulkan -> GLES -> nullptr. Vulkan is NOT_IMPLEMENTED in P01.
IRenderer* createRenderer() noexcept {
    // Vulkan not implemented — declared honestly.
    CapabilityManager::instance().setState(
        Capability::Renderer_Vulkan, CapabilityState::NotImplemented,
        "Vulkan backend pending — spec §5");
    auto* gl = new GLESRenderer();
    if (gl->initialize() != Status::Ok) {
        delete gl;
        return nullptr;
    }
    return gl;
}

} // namespace cyber
