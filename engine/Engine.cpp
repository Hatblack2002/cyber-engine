// cyber-engine :: engine lifecycle implementation
#include "engine/Engine.hpp"
#include "engine/renderer/IRenderer.hpp"
#include "engine/scene/Scene.hpp"

#include <chrono>

namespace cyber {

Result<void> Engine::create(IRenderer* renderer, Scene* scene) noexcept {
    if (state_ == State::Ready || state_ == State::Booting) {
        return Result<void>::Ok();
    }
    state_ = State::Booting;
    CYBER_LOG_INFO("engine", "boot start");

    // Stage 1 — Diagnostics (static singleton, always ready)
    (void)Diagnostics::instance();

    // Stage 2 — Memory (static singleton, always ready)
    // SystemAllocator::instance() is initialized on first call.

    // Stage 3 — JobSystem
    if (auto s = JobSystem::instance().initialize(); s != Status::Ok) {
        CYBER_LOG_FATAL("engine", "JobSystem init failed");
        state_ = State::Failed;
        return Result<void>::Err(Status::Error);
    }
    CapabilityManager::instance().registerCapability(Capability::Shell, CapabilityState::Available,
        "Built-in cyber shell: help, version, capabilities, diagnostics, echo, clear");

    // Stage 4 — EventBus (static singleton, always ready)
    (void)EventBus::instance();

    // Stage 5 — CapabilityManager (already constructed as singleton)
    CapabilityManager::instance().registerCapability(Capability::Root, CapabilityState::Unavailable,
        "No root detection — not faking it");
    CapabilityManager::instance().registerCapability(Capability::NativePlugins, CapabilityState::Unavailable,
        "DISABLED in P01 per spec");

    // Stage 6 — Renderer
    renderer_ = renderer;
    if (renderer_) {
        if (renderer_->initialize() != Status::Ok) {
            CYBER_LOG_ERROR("engine", "Renderer init failed");
            state_ = State::Failed;
            return Result<void>::Err(Status::Error);
        }
    } else {
        CYBER_LOG_WARN("engine", "No renderer provided — headless mode");
    }

    // Stage 7 — Scene
    scene_ = scene;
    if (scene_) {
        if (auto s = scene_->initialize(); s != Status::Ok) {
            CYBER_LOG_ERROR("engine", "Scene init failed");
            state_ = State::Failed;
            return Result<void>::Err(Status::Error);
        }
    }

    // Stage 8 — UI is Android-side, not engine-side.

    // Stage 9 — Lua (spec §10) — NOT_IMPLEMENTED in P01
    CapabilityManager::instance().registerCapability(Capability::LuaVM, CapabilityState::NotImplemented,
        "Lua VM sandbox pending — spec §10");

    // Honest capability states
    CapabilityManager::instance().registerCapability(
        Capability::NetworkObservation,
        CapabilityState::NotImplemented,
        "No ARP/raw socket — spec §0 forbids faking");
    CapabilityManager::instance().registerCapability(
        Capability::ProcessInfo,
        CapabilityState::RequiresRoot,
        "Only Android public APIs in P01; /proc fully needs root");

    state_ = State::Ready;
    EventBus::instance().publish(events::EngineReady{});
    CYBER_LOG_INFO("engine", "boot complete — READY");
    return Result<void>::Ok();
}

Status Engine::destroy() noexcept {
    if (state_ == State::Destroyed) return Status::Ok;
    if (scene_) { scene_->shutdown(); scene_ = nullptr; }
    if (renderer_) { renderer_->shutdown(); renderer_ = nullptr; }
    JobSystem::instance().shutdown();
    EventBus::instance().clear();
    state_ = State::Destroyed;
    CYBER_LOG_INFO("engine", "destroyed");
    return Status::Ok;
}

Status Engine::onPause() noexcept {
    if (state_ != State::Ready) return Status::Ok;
    state_ = State::Paused;
    if (renderer_) renderer_->onPause();
    EventBus::instance().publish(events::PauseRequested{});
    CYBER_LOG_INFO("engine", "paused");
    return Status::Ok;
}

Status Engine::onResume() noexcept {
    if (state_ != State::Paused) return Status::Ok;
    if (renderer_) renderer_->onResume();
    state_ = State::Ready;
    EventBus::instance().publish(events::ResumeRequested{});
    CYBER_LOG_INFO("engine", "resumed");
    return Status::Ok;
}

double Engine::tick() noexcept {
    if (state_ != State::Ready) return 0.0;
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    double deltaSec = 0.0;
    if (lastTickNs_ > 0) {
        deltaSec = static_cast<double>(now - lastTickNs_) / 1e9;
    }
    lastTickNs_ = now;
    EventBus::instance().publish(events::FrameBegin{frameNumber_, deltaSec});
    if (scene_) scene_->update(deltaSec);
    EventBus::instance().publish(events::FrameEnd{frameNumber_, deltaSec});
    return deltaSec;
}

Result<void> Engine::render() noexcept {
    if (state_ != State::Ready) return Result<void>::Err(Status::BadState);
    if (!renderer_) return Result<void>::Err(Status::Unavailable);
    auto r = renderer_->render(scene_);
    if (r.ok()) ++frameNumber_;
    return r;
}

} // namespace cyber
