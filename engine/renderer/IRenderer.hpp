// cyber-engine :: renderer interface + factory (spec §5)
// Order: Vulkan -> GLES -> ENGINE_GRAPHICS_UNAVAILABLE
#pragma once
#include "engine/core/Types.hpp"

namespace cyber {

class Scene;

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual Status initialize() noexcept = 0;
    virtual Status shutdown() noexcept = 0;
    virtual Status onPause() noexcept = 0;
    virtual Status onResume() noexcept = 0;
    virtual Status onSurfaceCreated(void* nativeWindow) noexcept = 0;
    virtual Status onSurfaceDestroyed() noexcept = 0;
    virtual Status onSurfaceResized(int32_t w, int32_t h) noexcept = 0;
    virtual Result<void> render(Scene* scene) noexcept = 0;
    virtual const char* backendName() const noexcept = 0;
    virtual int64_t frameCount() const noexcept = 0;
    virtual double lastFrameMs() const noexcept = 0;
};

// Factory: tries Vulkan first, then GLES, then nullptr (engine still boots headless).
// spec §5: "No `tryInitialize(); ignoreFailure();`" — failures are observable via CapabilityManager.
[[nodiscard]] IRenderer* createRenderer() noexcept;

} // namespace cyber
