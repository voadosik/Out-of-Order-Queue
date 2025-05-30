//#include <cstddef>
//
//#include <atomic>
//#include <chrono>
//#include <iostream>
//#include <vector>
//#include <thread>
//#include <utility>
//
//#include "test-common.hpp"
//
//#include "queue.hpp"
//
//static constexpr std::size_t timeout_ms = 1000;
//
//template<typename F, typename... Args>
//static bool test_with_simple_task(F&& task, Args &&... args) {
//    std::atomic<bool> done{ false };
//    Queue queue;
//
//    queue.enqueue(std::forward<F>(task), std::forward<Args>(args)...);
//
//    auto worker = std::thread([&queue, &done]() {
//        queue.serve();
//        done.store(true);
//        });
//
//    wait_for_time_or_done(done, timeout_ms);
//
//    if (!done.load()) {
//        PRINT_INDENTED("Worker thread did not exit in time");
//        return false;
//    }
//
//    worker.join();
//    return true;
//}
//
//TEST_CASE(construct_destruct, "just construct and destruct the queue") {
//    Queue queue;
//
//    return true;
//};
//
//TEST_CASE(no_work, "create a worker thread but do not enqueue any tasks, the thread should exit") {
//    std::atomic<bool> done{ false };
//    Queue queue;
//
//    auto worker = std::thread([&queue, &done]() {
//        queue.serve();
//        done.store(true);
//        });
//
//    wait_for_time_or_done(done, timeout_ms);
//
//    if (!done.load()) {
//        PRINT_INDENTED("Worker thread did not exit in time");
//        return false;
//    }
//
//    worker.join();
//    return true;
//}
//
//TEST_CASE(task_done, "create a worker thread and enqueue a trivial task, the thread should exit after the task is done") {
//    return test_with_simple_task([]() {
//        PRINT_INDENTED("Trivial task executed");
//        }, writes(), reads());
//}
//
//TEST_CASE(task_that_reads, "create a worker thread and enqueue a task that reads a resource, the thread should exit after the task is done") {
//    const resource_id resource = 0;
//    return test_with_simple_task([]() {
//        PRINT_INDENTED("Task executed, reading resource " << resource);
//        }, writes(), reads(resource));
//}
//
//TEST_CASE(task_that_reads_multiple, "create a worker thread and enqueue a task that reads multiple resources, the thread should exit after the task is done") {
//    constexpr std::size_t resources = 32;
//    return[]<std::size_t... I>(std::index_sequence<I...>) {
//        return test_with_simple_task([]() {
//            PRINT_INDENTED("Task executed, reading 32 resources");
//            }, writes(), reads(static_cast<resource_id>(I)...));
//    }(std::make_index_sequence<resources>{});
//}
//
//TEST_CASE(task_that_writes, "create a worker thread and enqueue a task that writes a resource, the thread should exit after the task is done") {
//    const resource_id resource = 0;
//    return test_with_simple_task([]() {
//        PRINT_INDENTED("Task executed, writing resource " << resource);
//        }, writes(resource), reads());
//}
//
//TEST_CASE(task_that_writes_multiple, "create a worker thread and enqueue a task that writes multiple resources, the thread should exit after the task is done") {
//    constexpr std::size_t resources = 32;
//    return[]<std::size_t... I>(std::index_sequence<I...>) {
//        return test_with_simple_task([]() {
//            PRINT_INDENTED("Task executed, writing 32 resources");
//            }, writes(static_cast<resource_id>(I)...), reads());
//    }(std::make_index_sequence<resources>{});
//}
//
//TEST_CASE(task_that_writes_and_reads, "create a worker thread and enqueue a task that writes and reads a resource, the thread should exit after the task is done") {
//    const resource_id resource1 = 0;
//    const resource_id resource2 = 1;
//    return test_with_simple_task([]() {
//        PRINT_INDENTED("Task executed, writing resource " << resource1 << " and reading resource " << resource2);
//        }, writes(resource1), reads(resource2));
//}
//
//TEST_CASE(task_that_writes_and_reads_multiple, "create a worker thread and enqueue a task that writes and reads multiple resources, the thread should exit after the task is done") {
//    constexpr std::size_t resources = 32;
//    return[=]<std::size_t... I>(std::index_sequence<I...>) {
//        return test_with_simple_task([]() {
//            PRINT_INDENTED("Task executed, writing 32 resources and reading 32 resources");
//            }, writes(static_cast<resource_id>(I)...), reads(static_cast<resource_id>(resources + I)...));
//    }(std::make_index_sequence<resources>{});
//}
//
//TEST_CASE(long_task_many_threads, "enqueue a long running task, and create many worker threads") {
//    constexpr std::size_t workers = 64;
//    constexpr std::size_t task_length_ms = 1000;
//
//    std::atomic<bool> done{ false };
//    std::vector<std::thread> threads;
//
//    Queue queue;
//
//    queue.enqueue([=, &done]() {
//        PRINT_INDENTED("Long running task started");
//        std::this_thread::sleep_for(std::chrono::milliseconds(task_length_ms));
//        PRINT_INDENTED("Long running task finished");
//        done.store(true);
//        }, writes(), reads());
//
//    for (std::size_t i = 0; i < workers; ++i) {
//        threads.emplace_back([&queue]() {
//            queue.serve();
//            });
//    }
//
//    wait_for_time_or_done(done, task_length_ms + timeout_ms);
//
//    if (!done.load()) {
//        PRINT_INDENTED("Worker thread did not exit in time");
//        return false;
//    }
//
//    for (auto& thread : threads) {
//        if (thread.joinable()) {
//            thread.join();
//        }
//    }
//
//    return true;
//}
//
//TEST_CASE(single_worker_many_tasks, "enqueue many tasks and create a single worker thread") {
//    constexpr std::size_t tasks = 1000;
//
//    std::atomic<std::size_t> done_tasks{ 0 };
//    std::atomic<bool> done{ false };
//
//    Queue queue;
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        queue.enqueue([=, &done_tasks]() {
//            (void)done_tasks.fetch_add(1);
//            }, writes(), reads());
//    }
//
//    auto worker = std::thread([&queue, &done]() {
//        queue.serve();
//        done.store(true);
//        });
//
//    wait_for_time_or_done(done, timeout_ms);
//
//    if (!done.load()) {
//        PRINT_INDENTED("Worker thread did not exit in time");
//        return false;
//    }
//
//    worker.join();
//
//    if (done_tasks.load() != tasks) {
//        PRINT_INDENTED("Not all tasks were executed, expected " << tasks << " but got " << done_tasks.load());
//        return false;
//    }
//
//    return true;
//}
//
//int main() {
//    std::size_t failed = 0;
//    std::size_t total = 0;
//
//
//    ++total;
//    if (!construct_destruct()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!no_work()) {
//        ++failed;
//    }
//    ++total;
//    if (!task_done()) {
//        ++failed;
//    }
//    ++total;
//    if (!task_that_reads()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_reads_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes_and_reads()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes_and_reads_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!single_worker_many_tasks()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!long_task_many_threads()) {
//        ++failed;
//    }
//
//    RESET_INDENT();
//    PRINT_INDENTED();
//
//    if (failed == 0) {
//        PRINT_INDENTED("All test cases passed");
//    }
//    else {
//        PRINT_INDENTED("Failed " << failed << " out of " << total << " tests");
//    }
//}
