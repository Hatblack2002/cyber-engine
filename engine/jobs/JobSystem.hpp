// cyber-engine :: jobs (spec §3 — worker pool)
#pragma once
#include "engine/core/Types.hpp"
#include "engine/diagnostics/Diagnostics.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <vector>

namespace cyber {

// Minimal worker pool. Jobs are std::function<void()>; no priorities, no dependencies in P01.
// All tasks are fire-and-forget; if a result is needed, use a promise/future inside the task.
class JobSystem {
public:
    static JobSystem& instance() noexcept {
        static JobSystem s;
        return s;
    }

    Status initialize(uint32_t workerCount = 0) noexcept {
        if (running_.load()) return Status::Ok;
        if (workerCount == 0) workerCount = std::max(1u, std::thread::hardware_concurrency());
        workers_.reserve(workerCount);
        for (uint32_t i = 0; i < workerCount; ++i) {
            workers_.emplace_back([this] { workerMain(); });
        }
        running_.store(true);
        CYBER_LOG_INFO("jobs", "JobSystem started with N workers");
        return Status::Ok;
    }

    Status shutdown() noexcept {
        if (!running_.load()) return Status::Ok;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            running_.store(false);
        }
        cv_.notify_all();
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
        workers_.clear();
        return Status::Ok;
    }

    JobId submit(std::function<void()> task) noexcept {
        if (!task) return JobId{0};
        JobId id;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            id = JobId{nextId_.fetch_add(1, std::memory_order_relaxed)};
            queue_.push({id, std::move(task)});
        }
        cv_.notify_one();
        return id;
    }

    size_t pending() const noexcept {
        std::lock_guard<std::mutex> lk(mtx_);
        return queue_.size();
    }

    uint32_t workerCount() const noexcept {
        return static_cast<uint32_t>(workers_.size());
    }

private:
    JobSystem() = default;
    ~JobSystem() { shutdown(); }

    struct JobEntry {
        JobId id{};
        std::function<void()> fn;
    };

    void workerMain() noexcept {
        for (;;) {
            JobEntry entry;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this] { return !running_.load() || !queue_.empty(); });
                if (!running_.load() && queue_.empty()) return;
                if (queue_.empty()) continue;
                entry = std::move(queue_.front());
                queue_.pop();
            }
            if (entry.fn) {
                entry.fn();
            }
        }
    }

    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<JobEntry> queue_;
    std::vector<std::thread> workers_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> nextId_{1};
};

} // namespace cyber
