// cyber-engine :: renderer factory stub for HOST builds (no EGL/GLES).
// On Android, GLESRenderer.cpp provides the real factory. On host, we return nullptr
// and the engine boots headless — useful for tests.
#if defined(__ANDROID__)
#  error "Host stub must not be compiled for Android."
#endif

#include "engine/renderer/IRenderer.hpp"
#include "engine/capabilities/CapabilityManager.hpp"

namespace cyber {

IRenderer* createRenderer() noexcept {
    CapabilityManager::instance().setState(
        Capability::Renderer_Vulkan, CapabilityState::NotImplemented,
        "Vulkan backend pending");
    CapabilityManager::instance().setState(
        Capability::Renderer_GLES, CapabilityState::Unavailable,
        "GLES/EGL not available on host");
    return nullptr;
}

} // namespace cyber
