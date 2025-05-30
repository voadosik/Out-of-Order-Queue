//#include <cstddef>
//
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
//static constexpr std::size_t timeout_ms = 300;
//
//TEST_CASE(write_write, "test a complex write-write dependency between multiple tasks") {
//    constexpr std::size_t task_length_ms = 100;
//    constexpr std::size_t tasks_delay_ms = 100;
//    constexpr std::size_t tasks = 20;
//
//    constexpr std::size_t resources = 1024;
//
//    Queue queue;
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        [=, &queue] <std::size_t... I>(std::index_sequence<I...>) {
//            queue.enqueue([=]() {
//                std::this_thread::sleep_for(std::chrono::milliseconds(task_length_ms));
//                }, writes(static_cast<resource_id>((I + i * 7) % resources)...), reads());
//        }(std::make_index_sequence<resources>{});
//    }
//
//    std::vector<std::thread> threads;
//    threads.reserve(tasks);
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        threads.emplace_back([&queue, &start]() {
//            std::this_thread::sleep_until(start);
//            queue.serve();
//            });
//    }
//
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    const auto elapsed = std::chrono::steady_clock::now() - start;
//    const std::size_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
//
//    if (elapsed_ms < task_length_ms * tasks) {
//        PRINT_INDENTED("Tasks finished too early, expected at least " << task_length_ms * tasks << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    if (elapsed_ms > task_length_ms * tasks + timeout_ms) {
//        PRINT_INDENTED("Tasks finished too late, expected at most " << task_length_ms * tasks + timeout_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(write_read, "test a simple write-read dependency between multiple tasks") {
//    constexpr std::size_t task_length_ms = 100;
//    constexpr std::size_t tasks_delay_ms = 100;
//    constexpr std::size_t tasks = 20;
//
//    constexpr std::size_t resources = 1024;
//
//    Queue queue;
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        [=, &queue] <std::size_t... I>(std::index_sequence<I...>) {
//            queue.enqueue([=]() {
//                std::this_thread::sleep_for(std::chrono::milliseconds(task_length_ms));
//                }, writes(static_cast<resource_id>((i + 1) * resources + (I + i * 7) % resources)...), reads(static_cast<resource_id>(i * resources + (I + i * 7) % resources)...));
//        }(std::make_index_sequence<resources>{});
//    }
//
//    std::vector<std::thread> threads;
//    threads.reserve(tasks);
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        threads.emplace_back([&queue, &start]() {
//            std::this_thread::sleep_until(start);
//            queue.serve();
//            });
//    }
//
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    const auto elapsed = std::chrono::steady_clock::now() - start;
//    const std::size_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
//
//    if (elapsed_ms < task_length_ms * tasks) {
//        PRINT_INDENTED("Tasks finished too early, expected at least " << task_length_ms * tasks << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    if (elapsed_ms > task_length_ms * tasks + timeout_ms) {
//        PRINT_INDENTED("Tasks finished too late, expected at most " << task_length_ms * tasks + timeout_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(read_write, "test a simple read-write dependency between multiple tasks") {
//    constexpr std::size_t task_length_ms = 100;
//    constexpr std::size_t tasks_delay_ms = 100;
//    constexpr std::size_t tasks = 20;
//
//    constexpr std::size_t resources = 1024;
//
//    Queue queue;
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        [=, &queue] <std::size_t... I>(std::index_sequence<I...>) {
//            queue.enqueue([=]() {
//                std::this_thread::sleep_for(std::chrono::milliseconds(task_length_ms));
//                }, writes(static_cast<resource_id>(i * resources + (I + i * 7) % resources)...), reads(static_cast<resource_id>((i + 1) * resources + (I + i * 7) % resources)...));
//        }(std::make_index_sequence<resources>{});
//    }
//
//    std::vector<std::thread> threads;
//    threads.reserve(tasks);
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        threads.emplace_back([&queue, &start]() {
//            std::this_thread::sleep_until(start);
//            queue.serve();
//            });
//    }
//
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    const auto elapsed = std::chrono::steady_clock::now() - start;
//    const std::size_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
//
//    if (elapsed_ms < task_length_ms * tasks) {
//        PRINT_INDENTED("Tasks finished too early, expected at least " << task_length_ms * tasks << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    if (elapsed_ms > task_length_ms * tasks + timeout_ms) {
//        PRINT_INDENTED("Tasks finished too late, expected at most " << task_length_ms * tasks + timeout_ms << "ms but got " << elapsed_ms);
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
//    ++total;
//    if (!write_write()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!write_read()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!read_write()) {
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
