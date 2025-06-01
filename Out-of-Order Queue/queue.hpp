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

    mutable std::mutex mtx;
    std::condition_variable ready;
    std::queue<std::shared_ptr<TaskControl>> ready_tasks;
    size_t unfinished_tasks = 0;
    size_t waiting_workers = 0;

    std::unordered_map<resource_id, std::weak_ptr<TaskControl>> last_writer;
    std::unordered_map<resource_id, std::weak_ptr<TaskControl>> last_task;
};


struct Queue::TaskControl {
    std::function<void()> task;
    std::atomic<size_t> dependency_count{ 0 };
    std::vector<std::shared_ptr<TaskControl>> dependents;
    bool finished = false;

    TaskControl(std::function<void()>);
};

Queue::TaskControl::TaskControl(std::function<void()> t)
    : task(std::move(t)) {
}


template<std::invocable Func, std::ranges::input_range WRange, std::ranges::input_range RRange>
    requires std::convertible_to<std::ranges::range_value_t<WRange>, resource_id>
&& std::convertible_to<std::ranges::range_value_t<RRange>, resource_id>
void Queue::enqueue(Func&& task, WRange&& writes, RRange&& reads) {
    std::set<resource_id> write_set;
    std::set<resource_id> read_set;

    for (auto&& w : writes) write_set.insert(static_cast<resource_id>(w));
    for (auto&& r : reads) read_set.insert(static_cast<resource_id>(r));

    std::shared_ptr<TaskControl> tc;
    {
        std::lock_guard<std::mutex> guard(mtx);
        tc = std::make_shared<TaskControl>(std::forward<Func>(task));
        unfinished_tasks++;

        std::set<std::shared_ptr<TaskControl>> dependencies;

        for (resource_id r : write_set) {
            if (auto it = last_task.find(r); it != last_task.end()) {
                if (auto last_task = it->second.lock()) {
                    if (!last_task->finished) {
                        dependencies.insert(last_task);
                    }
                }
            }

            last_writer[r] = tc;
            last_task[r] = tc;
        }

        for (resource_id r : read_set) {
            if (auto it = last_writer.find(r); it != last_writer.end()) {
                if (auto last_writer = it->second.lock()) {
                    if (!last_writer->finished) {
                        dependencies.insert(last_writer);
                    }
                }
            }

            last_task[r] = tc;
        }

        tc->dependency_count = dependencies.size();
        for (auto&& dep : dependencies) dep->dependents.push_back(tc);

        if (tc->dependency_count == 0) {
            ready_tasks.push(tc);
            if (waiting_workers > 0) ready.notify_one();
        }
    }
}


void Queue::serve() {
    while (true) {
        std::shared_ptr<TaskControl> tc;
        {
            std::unique_lock<std::mutex> serve_lock(mtx);
            ++waiting_workers;
            ready.wait(serve_lock, [this] { return !ready_tasks.empty() || unfinished_tasks == 0; });
            --waiting_workers;

            if (unfinished_tasks == 0 && ready_tasks.empty()) return;

            if (!ready_tasks.empty()) {
                tc = ready_tasks.front();
                ready_tasks.pop();
            }
            else continue;
        }

        tc->task();

        {
            std::lock_guard<std::mutex> guard(mtx);
            tc->finished = true;

            size_t new_ready = 0;
            for (auto&& dep : tc->dependents) {
                dep->dependency_count--;
                if (dep->dependency_count == 0) {
                    ready_tasks.push(dep);
                    new_ready++;
                }
            }
            tc->dependents.clear();
            unfinished_tasks--;

            if (new_ready > 0) 
                for (size_t i = 0; i < std::min(new_ready, waiting_workers); i++) 
                    ready.notify_one();
            else if (unfinished_tasks == 0) 
                ready.notify_all();
        }


    }
}


#endif // QUEUE_HPP