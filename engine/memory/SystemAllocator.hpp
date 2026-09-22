// cyber-engine :: memory (spec §3 — SystemAllocator)
#pragma once
#include "engine/core/Types.hpp"
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <algorithm>

namespace cyber {

// SystemAllocator: thin wrapper over malloc/free. Counts live allocations for diagnostics.
// No thread-safe free-list, no pool — that's later. Just honest accounting.
class SystemAllocator {
public:
    static SystemAllocator& instance() noexcept {
        static SystemAllocator s;
        return s;
    }

    void* allocate(size_t bytes, size_t align = alignof(std::max_align_t)) noexcept {
        void* p = nullptr;
        if (align <= alignof(std::max_align_t)) {
            p = std::malloc(bytes);
        } else {
            // over-align: use posix_memalign (portable across host + Android bionic).
            if (posix_memalign(&p, align, bytes) != 0) p = nullptr;
        }
        if (p) {
            liveBytes_.fetch_add(bytes, std::memory_order_relaxed);
            totalAllocs_.fetch_add(1, std::memory_order_relaxed);
            peakBytes_.store(std::max(peakBytes_.load(), liveBytes_.load()), std::memory_order_relaxed);
        }
        lastAllocBytes_ = bytes;
        return p;
    }

    void deallocate(void* p, size_t bytes = 0) noexcept {
        if (!p) return;
        if (bytes) liveBytes_.fetch_sub(bytes, std::memory_order_relaxed);
        std::free(p);
        totalFrees_.fetch_add(1, std::memory_order_relaxed);
    }

    // Stats for diagnostics panel — REAL numbers, no fake.
    struct Stats {
        size_t liveBytes = 0;
        size_t peakBytes = 0;
        size_t totalAllocs = 0;
        size_t totalFrees = 0;
    };
    Stats stats() const noexcept {
        return Stats{
            liveBytes_.load(std::memory_order_relaxed),
            peakBytes_.load(std::memory_order_relaxed),
            totalAllocs_.load(std::memory_order_relaxed),
            totalFrees_.load(std::memory_order_relaxed),
        };
    }

private:
    SystemAllocator() = default;
    std::atomic<size_t> liveBytes_{0};
    std::atomic<size_t> peakBytes_{0};
    std::atomic<size_t> totalAllocs_{0};
    std::atomic<size_t> totalFrees_{0};
    size_t lastAllocBytes_ = 0;
};

// RAII helper
template <typename T>
struct ScopedAlloc {
    T* ptr = nullptr;
    size_t count = 0;
    explicit ScopedAlloc(size_t n) : ptr(static_cast<T*>(SystemAllocator::instance().allocate(n * sizeof(T), alignof(T)))), count(n) {}
    ~ScopedAlloc() { if (ptr) SystemAllocator::instance().deallocate(ptr, count * sizeof(T)); }
    ScopedAlloc(const ScopedAlloc&) = delete;
    ScopedAlloc& operator=(const ScopedAlloc&) = delete;
    ScopedAlloc(ScopedAlloc&& o) noexcept : ptr(o.ptr), count(o.count) { o.ptr = nullptr; o.count = 0; }
    T* get() noexcept { return ptr; }
    const T* get() const noexcept { return ptr; }
    size_t size() const noexcept { return count; }
    explicit operator bool() const noexcept { return ptr != nullptr; }
};

} // namespace cyber
