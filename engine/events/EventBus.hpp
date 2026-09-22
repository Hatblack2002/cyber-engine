// cyber-engine :: events (spec §3 — typed EventBus)
#pragma once
#include "engine/core/Types.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <typeindex>
#include <any>
#include <algorithm>
#include <vector>

namespace cyber {

// Type-erased pub/sub. SubId can be used to unsubscribe.
// Uses std::any + std::type_index to dispatch by event type.
// No ordering guarantees across types; within a type, FIFO.
class EventBus {
public:
    static EventBus& instance() noexcept {
        static EventBus s;
        return s;
    }

    template <typename E>
    EventSubId subscribe(std::function<void(const E&)> handler) noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        EventSubId id{nextSub_.fetch_add(1, std::memory_order_relaxed)};
        auto& vec = subs_[std::type_index(typeid(E))];
        vec.push_back({id, [h = std::move(handler)](const void* p) {
            h(*static_cast<const E*>(p));
        }});
        return id;
    }

    template <typename E>
    void unsubscribe(EventSubId id) noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = subs_.find(std::type_index(typeid(E)));
        if (it == subs_.end()) return;
        auto& vec = it->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [id](const Handler& h){ return h.id == id; }), vec.end());
    }

    template <typename E>
    void publish(const E& evt) noexcept {
        std::vector<Handler> copy;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto it = subs_.find(std::type_index(typeid(E)));
            if (it == subs_.end()) return;
            copy = it->second;
        }
        for (auto& h : copy) {
            h.fn(static_cast<const void*>(&evt));
        }
    }

    void clear() noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        subs_.clear();
    }

    size_t subscriberCount() const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        size_t n = 0;
        for (const auto& [_, vec] : subs_) n += vec.size();
        return n;
    }

private:
    EventBus() = default;

    struct Handler {
        EventSubId id{};
        std::function<void(const void*)> fn;
    };

    mutable std::mutex mtx_;
    std::unordered_map<std::type_index, std::vector<Handler>> subs_;
    std::atomic<uint64_t> nextSub_{1};
};

// Common event types used across the engine.
namespace events {
struct EngineReady {};
struct SurfaceCreated { int32_t width = 0; int32_t height = 0; };
struct SurfaceDestroyed {};
struct SurfaceResized { int32_t width = 0; int32_t height = 0; };
struct PauseRequested {};
struct ResumeRequested {};
struct FrameBegin { int64_t frameNumber = 0; double deltaSec = 0.0; };
struct FrameEnd { int64_t frameNumber = 0; double deltaSec = 0.0; };
struct CapabilityChanged { CapabilityId id{}; };
struct ShellOutput { std::string text; };
} // namespace events

} // namespace cyber
