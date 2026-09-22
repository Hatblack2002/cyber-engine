// cyber-engine :: diagnostics (spec §3 — ring buffer)
#pragma once
#include "engine/core/Types.hpp"
#include <array>
#include <atomic>
#include <cstring>
#include <cstdio>
#include <chrono>

namespace cyber {

enum class DiagLevel : uint8_t { Info = 0, Warn, Error, Fatal };

struct DiagEntry {
    int64_t     monotonicMs = 0;
    DiagLevel   level = DiagLevel::Info;
    char        tag[16]{};
    char        message[160]{};
};

// Fixed-capacity ring buffer. Lock-free single producer is enough for engine boot path.
// For multi-threaded loggers, the JobSystem worker threads will funnel through the JobSystem.
class Diagnostics {
public:
    static constexpr size_t kCapacity = 256;

    static Diagnostics& instance() noexcept {
        static Diagnostics s;
        return s;
    }

    void log(DiagLevel level, StrView tag, StrView msg) noexcept {
        size_t idx = head_.fetch_add(1, std::memory_order_relaxed) % kCapacity;
        DiagEntry& e = ring_[idx];
        e.monotonicMs = nowMs();
        e.level = level;
        copyStr(e.tag, tag, sizeof(e.tag));
        copyStr(e.message, msg, sizeof(e.message));
    }

    void info(StrView tag, StrView msg) noexcept { log(DiagLevel::Info, tag, msg); }
    void warn(StrView tag, StrView msg) noexcept { log(DiagLevel::Warn, tag, msg); }
    void error(StrView tag, StrView msg) noexcept { log(DiagLevel::Error, tag, msg); }
    void fatal(StrView tag, StrView msg) noexcept { log(DiagLevel::Fatal, tag, msg); }

    Span<const DiagEntry> snapshot(Span<DiagEntry> out) const noexcept {
        // Returns a stable snapshot; we copy up to min(out.size, kCapacity) entries in order.
        size_t total = head_.load(std::memory_order_relaxed);
        size_t count = out.size() < kCapacity ? out.size() : kCapacity;
        if (count > total) count = total;
        for (size_t i = 0; i < count; ++i) {
            // most-recent-first ordering
            size_t src = (total - 1 - i) % kCapacity;
            out[i] = ring_[src];
        }
        return Span<const DiagEntry>(out.data, count);
    }

    size_t totalEntries() const noexcept { return head_.load(std::memory_order_relaxed); }

private:
    Diagnostics() = default;
    static int64_t nowMs() noexcept {
        // steady_clock — monotonic, suitable for diagnostics timestamps.
        auto ns = std::chrono::steady_clock::now().time_since_epoch().count();
        return static_cast<int64_t>(ns / 1000000);
    }
    static void copyStr(char* dst, StrView src, size_t cap) noexcept {
        size_t n = src.size() < cap - 1 ? src.size() : cap - 1;
        std::memcpy(dst, src.data(), n);
        dst[n] = 0;
    }
    std::array<DiagEntry, kCapacity> ring_{};
    std::atomic<size_t> head_{0};
};

#define CYBER_LOG_INFO(tag, msg) ::cyber::Diagnostics::instance().info(tag, msg)
#define CYBER_LOG_WARN(tag, msg) ::cyber::Diagnostics::instance().warn(tag, msg)
#define CYBER_LOG_ERROR(tag, msg) ::cyber::Diagnostics::instance().error(tag, msg)
#define CYBER_LOG_FATAL(tag, msg) ::cyber::Diagnostics::instance().fatal(tag, msg)

} // namespace cyber
