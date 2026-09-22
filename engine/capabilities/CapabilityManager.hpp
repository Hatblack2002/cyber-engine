// cyber-engine :: capabilities (spec §3, §6 — single source of truth)
#pragma once
#include "engine/core/Types.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

namespace cyber {

// Spec §0 — estados legales:
// AVAILABLE | PARTIAL | UNAVAILABLE | REQUIRES_PERMISSION | REQUIRES_ROOT |
// REQUIRES_HARDWARE | NOT_IMPLEMENTED | ERROR
enum class CapabilityState : uint8_t {
    Available        = 0,
    Partial          = 1,
    Unavailable      = 2,
    RequiresPermission = 3,
    RequiresRoot     = 4,
    RequiresHardware = 5,
    NotImplemented   = 6,
    Error            = 7,
};

inline const char* capabilityStateLabel(CapabilityState s) noexcept {
    switch (s) {
        case CapabilityState::Available:          return "AVAILABLE";
        case CapabilityState::Partial:            return "PARTIAL";
        case CapabilityState::Unavailable:        return "UNAVAILABLE";
        case CapabilityState::RequiresPermission: return "REQUIRES_PERMISSION";
        case CapabilityState::RequiresRoot:       return "REQUIRES_ROOT";
        case CapabilityState::RequiresHardware:   return "REQUIRES_HARDWARE";
        case CapabilityState::NotImplemented:     return "NOT_IMPLEMENTED";
        case CapabilityState::Error:              return "ERROR";
    }
    return "?";
}

// Known capabilities (spec §6). Pinned enum for compile-time safety.
enum class Capability : uint16_t {
    Renderer_Vulkan   = 1,
    Renderer_GLES     = 2,
    AppPrivateStorage = 3,
    SharedStorage     = 4,
    SAF              = 5, // Storage Access Framework
    Root              = 6,
    NetworkObservation = 7,
    ProcessInfo       = 8,
    Terminal          = 9,
    LuaVM             = 10,
    NativePlugins     = 11, // DISABLED in P01 per spec §6
    Filesystem        = 12,
    SystemInfo        = 13,
    Scene3D           = 14,
    Camera            = 15,
    Touch             = 16,
    Picking           = 17,
    Shell             = 18,
    UIDashboard       = 19,
};

inline const char* capabilityName(Capability c) noexcept {
    switch (c) {
        case Capability::Renderer_Vulkan:   return "Renderer.Vulkan";
        case Capability::Renderer_GLES:     return "Renderer.GLES";
        case Capability::AppPrivateStorage: return "Storage.AppPrivate";
        case Capability::SharedStorage:    return "Storage.Shared";
        case Capability::SAF:              return "Storage.SAF";
        case Capability::Root:             return "Root";
        case Capability::NetworkObservation:return "Network.Observation";
        case Capability::ProcessInfo:      return "Process.Info";
        case Capability::Terminal:         return "Terminal";
        case Capability::LuaVM:            return "Lua.VM";
        case Capability::NativePlugins:    return "NativePlugins";
        case Capability::Filesystem:       return "Filesystem";
        case Capability::SystemInfo:       return "System.Info";
        case Capability::Scene3D:          return "Scene3D";
        case Capability::Camera:           return "Camera";
        case Capability::Touch:           return "Touch";
        case Capability::Picking:          return "Picking";
        case Capability::Shell:           return "Shell";
        case Capability::UIDashboard:     return "UI.Dashboard";
    }
    return "?";
}

struct CapabilityInfo {
    Capability     cap{};
    CapabilityState state = CapabilityState::NotImplemented;
    std::string    note; // human-readable explanation, never data
};

// Single source of truth. UI queries this; modules set their own state honestly.
class CapabilityManager {
public:
    static CapabilityManager& instance() noexcept {
        static CapabilityManager s;
        return s;
    }

    void registerCapability(Capability c, CapabilityState initial = CapabilityState::NotImplemented,
                            std::string note = {}) noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        caps_[static_cast<uint16_t>(c)] = CapabilityInfo{c, initial, std::move(note)};
    }

    void setState(Capability c, CapabilityState s, std::string note = {}) noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        auto& info = caps_[static_cast<uint16_t>(c)];
        info.cap = c;
        info.state = s;
        if (!note.empty()) info.note = std::move(note);
    }

    CapabilityState state(Capability c) const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = caps_.find(static_cast<uint16_t>(c));
        return it == caps_.end() ? CapabilityState::NotImplemented : it->second.state;
    }

    CapabilityInfo info(Capability c) const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = caps_.find(static_cast<uint16_t>(c));
        if (it == caps_.end()) return CapabilityInfo{c, CapabilityState::NotImplemented, {}};
        return it->second;
    }

    std::vector<CapabilityInfo> all() const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        std::vector<CapabilityInfo> out;
        out.reserve(caps_.size());
        for (const auto& [_, info] : caps_) out.push_back(info);
        std::sort(out.begin(), out.end(),
            [](const CapabilityInfo& a, const CapabilityInfo& b){ return static_cast<uint16_t>(a.cap) < static_cast<uint16_t>(b.cap); });
        return out;
    }

private:
    CapabilityManager() = default;
    mutable std::mutex mtx_;
    std::unordered_map<uint16_t, CapabilityInfo> caps_;
};

} // namespace cyber
