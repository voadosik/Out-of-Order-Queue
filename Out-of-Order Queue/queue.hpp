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
#include <unordered_set>

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
    struct Task;

    void track_dependencies(std::shared_ptr<Task>&);

    mutable std::mutex mtx;
    std::condition_variable ready;
    std::queue<std::shared_ptr<Task>> ready_tasks;
    std::vector<std::shared_ptr<Task>> waiting_tasks;

    std::unordered_map<resource_id, int> readers;
    std::unordered_set<resource_id> writers;

    int executing = 0;
    std::atomic<bool> off = false;
};


struct Queue::Task {
    std::function<void()> task;
    std::unordered_set<resource_id> writes;
    std::unordered_set<resource_id> reads;
    int pending = 0;
};


void Queue::track_dependencies(std::shared_ptr<Task>& task) {
    for (resource_id rid : task->writes) 
        if (writers.count(rid) || readers.count(rid))
            task->pending++;
    
    for (resource_id rid : task->reads)
        if (writers.count(rid))
            task->pending++;
}

template<std::invocable Func, std::ranges::input_range WRange, std::ranges::input_range RRange>
    requires std::convertible_to<std::ranges::range_value_t<WRange>, resource_id>
&& std::convertible_to<std::ranges::range_value_t<RRange>, resource_id>
void Queue::enqueue(Func&& task, WRange&& writes, RRange&& reads) {
    auto t = std::make_shared<Task>();
    t->task = std::forward<Func>(task);
    t->writes.insert(std::ranges::begin(writes), std::ranges::end(writes));
    t->reads.insert(std::ranges::begin(reads), std::ranges::end(reads));
    
    {
        std::lock_guard<std::mutex> guard(mtx);
        track_dependencies(t);
        if (t->pending == 0) {
            ready_tasks.push(t);
            ready.notify_one();
        }
        else
            waiting_tasks.push_back(t);
    }
}

void Queue::serve() {
    while (true) {
        std::shared_ptr<Task> t;
        {
            std::unique_lock<std::mutex> serve_lock(mtx);
            ready.wait(serve_lock, [this] {
                return !ready_tasks.empty() || off;
                });

            if (off && ready_tasks.empty()) return;
            t = ready_tasks.front();
            ready_tasks.pop();
            executing++;
            for (auto& r : t->writes) writers.insert(r);
            for (auto& r : t->reads) readers[r]++;
        }

        t->task();

        {
            std::lock_guard<std::mutex> guard(mtx);
            for (auto r : t->writes) writers.erase(r);
            for (auto r : t->reads) if (--readers[r] == 0) readers.erase(r);

            --executing;
            for(auto& t : waiting_tasks)
                if (t->pending > 0 && --t->pending == 0) {
                    ready_tasks.push(t);
                    ready.notify_one();
                }

            waiting_tasks.erase(
                std::remove_if(
                    waiting_tasks.begin(),
                    waiting_tasks.end(),
                    [](const auto& t) { return t->pending == 0; }
                ), waiting_tasks.end());

            if (executing == 0 && ready_tasks.empty() && waiting_tasks.empty()) {
                off = true;
                ready.notify_all();
            }
        }
    }
}

#endif // QUEUE_HPP
