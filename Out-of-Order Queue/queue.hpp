//#ifndef QUEUE_HPP
//#define QUEUE_HPP
//
//#include <atomic>
//#include <condition_variable>
//#include <cstdint>
//#include <functional>
//#include <memory>
//#include <mutex>
//#include <queue>
//#include <ranges>
//#include <shared_mutex>
//#include <unordered_map>
//#include <unordered_set>
//#include <vector>
//#include <algorithm>
//
//using resource_id = std::uintptr_t;
//
//class Queue {
//public:
//    // Performs the initialization of the queue and exits
//    // This method is not allowed to block and is not needed to be thread-safe.
//    Queue();
//
//    // Performs cleanup of the queue. Is not needed to be thread-safe.
//    ~Queue(); // noexcept by default
//
//    // Queue is not copyable
//    Queue(const Queue&) = delete;
//    Queue& operator=(const Queue&) = delete;
//
//    // Queue is movable (not required to be thread-safe)
//    Queue(Queue&&) noexcept;
//    Queue& operator=(Queue&&) noexcept;
//
//    // Enqueues a new task with the given resource dependencies
//    //  to be processed by a worker thread when all the resources are available.
//    // This method is thread-safe and is not allowed to block.
//    template<std::invocable Func, std::ranges::input_range WRange, std::ranges::input_range RRange>
//        requires std::convertible_to<std::ranges::range_value_t<WRange>, resource_id>
//    && std::convertible_to<std::ranges::range_value_t<RRange>, resource_id>
//        void enqueue(Func&& task, WRange&& writes, RRange&& reads);
//
//    // Makes the current thread a worker thread and starts processing tasks
//    //  until the queue is empty. The method will exit when the queue is empty and
//    //  no tasks are currently being processed.
//    // This method is thread-safe and does not return until the queue is empty.
//    void serve();
//
//    struct Task;
//    struct ResourceState;
//
//    struct Impl;
//    std::unique_ptr<Impl> impl;
//};
//
//namespace std {
//    template<>
//    struct hash<std::shared_ptr<Queue::Task>> {
//        size_t operator()(const std::shared_ptr<Queue::Task>& ptr) const noexcept {
//            return hash<void*>{}(ptr.get());
//        }
//    };
//}
//
//struct Queue::Task {
//    std::function<void()> func;
//    std::atomic<int> dependency_count{ 0 };
//    std::mutex dependents_mutex;
//    std::vector<std::shared_ptr<Task>> dependents;
//
//    explicit Task(std::function<void()> f) : func(std::move(f)) {}
//};
//
//struct Queue::ResourceState {
//    std::shared_ptr<Task> last_write;
//    std::vector<std::shared_ptr<Task>> pending_reads;
//    std::mutex mutex;
//};
//
//struct Queue::Impl {
//    std::shared_mutex resources_mtx;
//    std::unordered_map<resource_id, ResourceState> resources;
//
//    std::mutex ready_mtx;
//    std::condition_variable ready_cv;
//    std::queue<std::shared_ptr<Task>> ready_queue;
//
//    std::atomic<int> task_count{ 0 };
//};
//
//Queue::Queue() : impl(std::make_unique<Impl>()) {}
//
//Queue::~Queue() = default;
//
//Queue::Queue(Queue&&) noexcept = default;
//Queue& Queue::operator=(Queue&&) noexcept = default;
//
//
//
//#endif // QUEUE_HPP

#ifndef QUEUE_HPP
#define QUEUE_HPP


#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <ranges>
#include <cstdint>

using resource_id = std::uintptr_t;

class Queue {
public:
    Queue() = default;
    ~Queue() {
        {
            std::unique_lock lock(ready_mutex_);
            shutdown_ = true;
        }
        ready_cv_.notify_all();
    }

    Queue(Queue&&) noexcept = delete;
    Queue& operator=(Queue&&) noexcept = delete;

    template<std::invocable Func, std::ranges::input_range WRange, std::ranges::input_range RRange>
        requires std::convertible_to<std::ranges::range_value_t<WRange>, resource_id>
    && std::convertible_to<std::ranges::range_value_t<RRange>, resource_id>
        void enqueue(Func&& task, WRange&& writes, RRange&& reads) {
        auto t = std::make_shared<Task>();
        t->func = std::forward<Func>(task);
        t->pending_dependencies.store(0);
        t->completed = false;

        std::unordered_set<std::shared_ptr<Task>> dependencies;

        // Process write resources
        for (const auto& w : writes) {
            std::unique_lock rslock(resource_states_mutex_);
            auto& rs = resource_states_[w];
            rslock.unlock();

            std::unique_lock wlock(rs.mutex);
            if (rs.last_write) {
                dependencies.insert(rs.last_write);
            }
            for (const auto& read_task : rs.active_reads) {
                dependencies.insert(read_task);
            }
            rs.active_reads.clear();
            rs.last_write = t;
        }

        // Process read resources
        for (const auto& r : reads) {
            std::unique_lock rslock(resource_states_mutex_);
            auto& rs = resource_states_[r];
            rslock.unlock();

            std::unique_lock rlock(rs.mutex);
            if (rs.last_write) {
                dependencies.insert(rs.last_write);
            }
            rs.active_reads.push_back(t);
        }

        t->pending_dependencies.store(dependencies.size());

        // Link dependencies
        for (const auto& D : dependencies) {
            std::unique_lock dlock(D->mutex);
            if (D->completed) {
                t->pending_dependencies.fetch_sub(1, std::memory_order_relaxed);
            }
            else {
                D->dependents.push_back(t);
            }
        }

        {
            std::unique_lock tlock(tasks_mutex_);
            tasks_.push_back(t);
        }
        active_tasks_.fetch_add(1, std::memory_order_relaxed);

        if (t->pending_dependencies.load(std::memory_order_relaxed) == 0) {
            std::unique_lock rlock(ready_mutex_);
            ready_queue_.push(t);
            rlock.unlock();
            ready_cv_.notify_one();
        }
    }

    void serve() {
        while (true) {
            std::shared_ptr<Task> task;
            {
                std::unique_lock rlock(ready_mutex_);
                ready_cv_.wait(rlock, [this] {
                    return !ready_queue_.empty() || active_tasks_.load(std::memory_order_relaxed) == 0 || shutdown_;
                    });

                if (shutdown_) {
                    break;
                }

                if (active_tasks_.load(std::memory_order_relaxed) == 0 && ready_queue_.empty()) {
                    break;
                }

                if (!ready_queue_.empty()) {
                    task = ready_queue_.front();
                    ready_queue_.pop();
                }
            }

            if (task) {
                try {
                    task->func();
                }
                catch (...) {}

                std::vector<std::shared_ptr<Task>> dependents;
                {
                    std::unique_lock tlock(task->mutex);
                    task->completed = true;
                    dependents.swap(task->dependents);
                }

                for (const auto& dependent : dependents) {
                    auto prev = dependent->pending_dependencies.fetch_sub(1, std::memory_order_acq_rel);
                    if (prev == 1) {
                        std::unique_lock rlock(ready_mutex_);
                        ready_queue_.push(dependent);
                        rlock.unlock();
                        ready_cv_.notify_one();
                    }
                }

                active_tasks_.fetch_sub(1, std::memory_order_relaxed);
            }
        }
    }

private:
    struct Task {
        std::function<void()> func;
        std::atomic<size_t> pending_dependencies;
        std::mutex mutex;
        bool completed = false;
        std::vector<std::shared_ptr<Task>> dependents;
    };

    struct ResourceState {
        std::mutex mutex;
        std::shared_ptr<Task> last_write;
        std::vector<std::shared_ptr<Task>> active_reads;
    };

    std::mutex resource_states_mutex_;
    std::unordered_map<resource_id, ResourceState> resource_states_;

    std::mutex ready_mutex_;
    std::condition_variable ready_cv_;
    std::queue<std::shared_ptr<Task>> ready_queue_;

    std::mutex tasks_mutex_;
    std::vector<std::shared_ptr<Task>> tasks_;

    std::atomic<size_t> active_tasks_ = 0;
    std::atomic<bool> shutdown_ = false;
};


#endif