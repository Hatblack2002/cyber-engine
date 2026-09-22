// cyber-engine :: Boot diagnostics (spec §0 — never hide a failure behind an empty screen)
// Ring buffer of boot stages. Each entry has a stage name, ok/fail flag, and
// optional reason. Queryable from the UI via JNI so the user can see exactly
// where the boot stopped.
#pragma once
#include "engine/core/Types.hpp"
#include <array>
#include <atomic>
#include <mutex>
#include <string>
#include <string_view>
#include <sstream>
#include <chrono>
#include <cstring>

namespace cyber {

enum class BootStage : uint8_t {
    ActivityCreated          = 0,
    JNILibraryLoaded         = 1,
    EngineCreated            = 2,
    RendererCreationStarted  = 3,
    EGLInitialization        = 4,
    GLESContextCreated       = 5,
    SurfaceCreated            = 6,
    RenderLoopStarted         = 7,
    LinuxBridgeInitialized   = 8,
    BusyBoxLocated            = 9,
    PTYInitialized            = 10,
    ShellSessionStarted       = 11,
};

inline const char* bootStageLabel(BootStage s) noexcept {
    switch (s) {
        case BootStage::ActivityCreated:         return "Activity created";
        case BootStage::JNILibraryLoaded:        return "JNI library loaded";
        case BootStage::EngineCreated:           return "Engine created";
        case BootStage::RendererCreationStarted: return "Renderer creation started";
        case BootStage::EGLInitialization:       return "EGL initialization";
        case BootStage::GLESContextCreated:      return "GLES context created";
        case BootStage::SurfaceCreated:          return "Surface created";
        case BootStage::RenderLoopStarted:       return "Render loop started";
        case BootStage::LinuxBridgeInitialized:  return "Linux bridge initialized";
        case BootStage::BusyBoxLocated:          return "BusyBox located";
        case BootStage::PTYInitialized:          return "PTY initialized";
        case BootStage::ShellSessionStarted:     return "Shell session started";
    }
    return "?";
}

struct BootEntry {
    int64_t     monotonicMs = 0;
    BootStage   stage = BootStage::ActivityCreated;
    bool        ok = true;
    char        reason[160]{};
};

class BootDiagnostics {
public:
    static constexpr size_t kCapacity = 64;

    static BootDiagnostics& instance() noexcept {
        static BootDiagnostics s;
        return s;
    }

    void mark(BootStage stage, bool ok, StrView reason = {}) noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        size_t idx = head_ % kCapacity;
        BootEntry& e = ring_[idx];
        e.monotonicMs = nowMs();
        e.stage = stage;
        e.ok = ok;
        copyStr(e.reason, reason, sizeof(e.reason));
        ++head_;
    }

    void ok(BootStage stage, StrView reason = {}) noexcept { mark(stage, true, reason); }
    void fail(BootStage stage, StrView reason) noexcept { mark(stage, false, reason); }

    std::string dump() const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        std::ostringstream os;
        size_t total = head_;
        size_t count = (total < kCapacity) ? total : kCapacity;
        for (size_t i = 0; i < count; ++i) {
            // Oldest first.
            size_t src = (total + kCapacity - count + i) % kCapacity;
            const BootEntry& e = ring_[src];
            os << "[" << (e.ok ? "BOOT" : "ERROR") << "] "
               << bootStageLabel(e.stage);
            if (!e.ok || e.reason[0]) {
                os << "  reason=" << e.reason;
            }
            os << "\n";
        }
        return os.str();
    }

    void clear() noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        head_ = 0;
        for (auto& e : ring_) { e = BootEntry{}; }
    }

    bool stageReached(BootStage stage) const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        for (size_t i = 0; i < kCapacity; ++i) {
            if (ring_[i].stage == stage && ring_[i].ok) return true;
        }
        return false;
    }

private:
    BootDiagnostics() = default;
    static int64_t nowMs() noexcept {
        auto ns = std::chrono::steady_clock::now().time_since_epoch().count();
        return static_cast<int64_t>(ns / 1000000);
    }
    static void copyStr(char* dst, StrView src, size_t cap) noexcept {
        size_t n = src.size() < cap - 1 ? src.size() : cap - 1;
        std::memcpy(dst, src.data(), n);
        dst[n] = 0;
    }

    mutable std::mutex mtx_;
    std::array<BootEntry, kCapacity> ring_{};
    size_t head_ = 0;
};

} // namespace cyber
