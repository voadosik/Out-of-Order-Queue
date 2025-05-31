#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstdint>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <vector>
#include <set>
#include <memory>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

using resource_id = std::uintptr_t;

class Queue {
public:

    // Performs the initialization of the queue and exits
    // This method is not allowed to block and is not needed to be thread-safe.
    Queue() = default;

    // Performs cleanup of the queue. Is not needed to be thread-safe.
    ~Queue() = default; // noexcept by default

    // Queue is not copyable
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;


    // Queue is movable (not required to be thread-safe)
    Queue(Queue&&) noexcept = default;
    Queue& operator=(Queue&&) noexcept = default;


    // Enqueues a new task with the given resource dependencies
    //  to be processed by a worker thread when all the resources are available.
    // This method is thread-safe and is not allowed to block.
    template<std::invocable Func, std::ranges::input_range WRange, std::ranges::input_range RRange>
        requires std::convertible_to<std::ranges::range_value_t<WRange>, resource_id>
    && std::convertible_to<std::ranges::range_value_t<RRange>, resource_id>
    void enqueue(Func&& task, WRange&& writes, RRange&& reads);


    // Makes the current thread a worker thread and starts processing tasks
    //  until the queue is empty. The method will exit when the queue is empty and
    //  no tasks are currently being processed.
    // This method is thread-safe and does not return until the queue is empty.
    void serve();

private:
    struct TaskControl;

    mutable std::mutex queue_mutex;
    std::condition_variable ready_cond;
    std::queue<std::shared_ptr<TaskControl>> ready_queue;
    size_t sequence_counter = 0;
    size_t unfinished_tasks = 0;
    size_t waiting_workers = 0;

    std::unordered_map<resource_id, std::weak_ptr<TaskControl>> last_writer_map;
    std::unordered_map<resource_id, std::weak_ptr<TaskControl>> last_task_map;
};


struct Queue::TaskControl {
    size_t sequence_number;
    std::function<void()> task;
    size_t dependency_count = 0;
    std::vector<std::shared_ptr<TaskControl>> dependents;
    bool finished = false;

    TaskControl(size_t, std::function<void()>);
};

Queue::TaskControl::TaskControl(size_t seq, std::function<void()> t)
    : sequence_number(seq), task(std::move(t)) {
}



template<std::invocable Func, std::ranges::input_range WRange, std::ranges::input_range RRange>
    requires std::convertible_to<std::ranges::range_value_t<WRange>, resource_id>
&& std::convertible_to<std::ranges::range_value_t<RRange>, resource_id>
void Queue::enqueue(Func&& task, WRange&& writes, RRange&& reads) {
    std::set<resource_id> write_set;
    std::set<resource_id> read_set;

    for (const auto& w : writes) {
        write_set.insert(static_cast<resource_id>(w));
    }

    for (const auto& r : reads) {
        read_set.insert(static_cast<resource_id>(r));
    }

    std::shared_ptr<TaskControl> tc;
    {
        std::lock_guard<std::mutex> guard(queue_mutex);
        tc = std::make_shared<TaskControl>(sequence_counter++, std::forward<Func>(task));
        unfinished_tasks++;

        std::set<std::shared_ptr<TaskControl>> dependencies;

        for (resource_id r : write_set) {
            if (auto it = last_task_map.find(r); it != last_task_map.end()) {
                if (auto last_task = it->second.lock()) {
                    if (!last_task->finished) {
                        dependencies.insert(last_task);
                    }
                }
            }

            /*if (auto it = last_writer_map.find(r); it != last_writer_map.end()) {
                if (auto last_writer = it->second.lock()) {
                    if (!last_writer->finished && dependencies.find(last_writer) == dependencies.end()) {
                        dependencies.insert(last_writer);
                    }
                }
            }*/

            last_writer_map[r] = tc;
            last_task_map[r] = tc;
        }

        for (resource_id r : read_set) {
            if (auto it = last_writer_map.find(r); it != last_writer_map.end()) {
                if (auto last_writer = it->second.lock()) {
                    if (!last_writer->finished) {
                        dependencies.insert(last_writer);
                    }
                }
            }

            last_task_map[r] = tc;
        }

        tc->dependency_count = dependencies.size();
        for (auto& dep : dependencies) {
            dep->dependents.push_back(tc);
        }

        if (tc->dependency_count == 0) {
            ready_queue.push(tc);
            if (waiting_workers > 0) {
                ready_cond.notify_one();
            }
        }
    }
}


void Queue::serve() {
    while (true) {
        std::shared_ptr<TaskControl> tc;
        {
            std::unique_lock<std::mutex> serve_lock(queue_mutex);
            ++waiting_workers;
            ready_cond.wait(serve_lock, [this] {
                return !ready_queue.empty() || unfinished_tasks == 0;
                });
            --waiting_workers;

            if (unfinished_tasks == 0 && ready_queue.empty()) {
                return;
            }

            if (!ready_queue.empty()) {
                tc = ready_queue.front();
                ready_queue.pop();
            }
            else {
                continue;
            }
        }

        if (tc)
            tc->task();

        {
            std::lock_guard<std::mutex> guard(queue_mutex);
            tc->finished = true;

            size_t new_ready = 0;
            for (auto& dep : tc->dependents) {
                dep->dependency_count--;
                if (dep->dependency_count == 0) {
                    ready_queue.push(dep);
                    new_ready++;
                }
            }
            tc->dependents.clear();
            unfinished_tasks--;

            if (new_ready > 0) {
                size_t to_wake = std::min(new_ready, waiting_workers);
                for (size_t i = 0; i < to_wake; i++) {
                    ready_cond.notify_one();
                }
            }
            else if (unfinished_tasks == 0) {
                ready_cond.notify_all();
            }
        }
    }
}


#endif // QUEUE_HPP