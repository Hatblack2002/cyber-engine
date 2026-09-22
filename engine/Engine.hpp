// cyber-engine :: engine (spec §3, §4, §8 — lifecycle)
#pragma once
#include "engine/core/Types.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include "engine/jobs/JobSystem.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/capabilities/CapabilityManager.hpp"

namespace cyber {

class IRenderer;
class Scene;

// Spec §8 boot sequence:
//   Diagnostics -> Memory -> JobSystem -> EventBus -> CapabilityManager
//   -> Renderer (Vulkan|GLES|fail) -> Scene -> UI -> Lua -> READY
// Each stage returns Result; failure aborts subsequent stages and the engine
// remains in state Failed. No "tryInitialize(); ignoreFailure();".
class Engine {
public:
    enum class State : uint8_t {
        Uninitialized = 0,
        Booting       = 1,
        Ready         = 2,
        Paused        = 3,
        Failed        = 4,
        Destroyed     = 5,
    };

    static Engine& instance() noexcept {
        static Engine s;
        return s;
    }

    // Boot all subsystems. Returns Ok only if every stage succeeded.
    Result<void> create(IRenderer* renderer = nullptr, Scene* scene = nullptr) noexcept;

    // Release everything (JobSystem drain, EventBus clear, renderer release).
    // Idempotent — safe to call multiple times.
    Status destroy() noexcept;

    // Lifecycle hooks (spec §31-32). SurfaceDestroyed != EngineDestroyed.
    Status onPause() noexcept;
    Status onResume() noexcept;

    State state() const noexcept { return state_; }
    IRenderer* renderer() const noexcept { return renderer_; }
    Scene*     scene() const noexcept { return scene_; }

    // Per-frame update. Returns deltaSec for diagnostics.
    double tick() noexcept;

    // Render one frame using the active renderer. Honest frame counter; never fake 60fps.
    Result<void> render() noexcept;

private:
    Engine() = default;
    ~Engine() { destroy(); }
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    State        state_ = State::Uninitialized;
    IRenderer*   renderer_ = nullptr;
    Scene*       scene_ = nullptr;
    int64_t      frameNumber_ = 0;
    int64_t      lastTickNs_ = 0;
};

} // namespace cyber
