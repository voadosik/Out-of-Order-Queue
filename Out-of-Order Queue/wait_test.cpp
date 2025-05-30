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
//template<typename FWrites, typename FReads, typename SWrites, typename SReads>
//bool common_wait_test(FWrites&& first_writes, FReads&& first_reads, SWrites&& second_writes, SReads&& second_reads, bool expects_done = true) {
//    constexpr std::size_t task_length_ms = 300;
//    constexpr std::size_t tasks_delay_ms = 50;
//    constexpr std::size_t tasks = 128;
//
//    std::atomic<bool> done{ false };
//    std::atomic<bool> found_error{ false };
//
//    Queue queue;
//
//    queue.enqueue([task_length_ms, &done]() {
//        std::this_thread::sleep_for(std::chrono::milliseconds(task_length_ms));
//        done.store(true);
//        }, std::forward<FWrites>(first_writes), std::forward<FReads>(first_reads));
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        queue.enqueue([&done, &found_error, expects_done]() {
//            if (done.load() != expects_done) {
//                found_error.store(true);
//            }
//            }, std::forward<SWrites>(second_writes), std::forward<SReads>(second_reads));
//    }
//
//    std::vector<std::thread> threads;
//    threads.reserve(tasks + 1);
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    for (std::size_t i = 0; i < tasks + 1; ++i) {
//        threads.emplace_back([&queue, start]() {
//            std::this_thread::sleep_until(start);
//            queue.serve();
//            });
//    }
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        queue.enqueue([&done, &found_error, expects_done]() {
//            if (done.load() != expects_done) {
//                found_error.store(true);
//            }
//            }, std::forward<SWrites>(second_writes), std::forward<SReads>(second_reads));
//    }
//
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    if (found_error.load()) {
//        PRINT_INDENTED("Found error: a reader task was executed before the writer task finished");
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(wait_test_write_read, "test that reader tasks are not executed before writer task is finished") {
//    return common_wait_test(writes(0), reads(), writes(), reads(0));
//}
//
//TEST_CASE(wait_test_non_write_read, "test that reader tasks of unrelated resources are executed before writer task is finished") {
//    return common_wait_test(writes(0), reads(), writes(), reads(1), false);
//}
//
//TEST_CASE(wait_test_write_read_multiple, "test that reader tasks are not executed before writer task is finished with multiple resources") {
//    return common_wait_test(writes(0, 1), reads(), reads(0, 1), writes()) &&
//        common_wait_test(writes(0, 1), reads(), reads(1, 0), writes()) &&
//        common_wait_test(writes(0, 1), reads(), reads(0, 2), writes()) &&
//        common_wait_test(writes(0, 1), reads(), reads(2, 0), writes());
//}
//
//TEST_CASE(wait_test_read_write, "test that writer tasks are not executed before reader task is finished") {
//    return common_wait_test(writes(), reads(0), writes(0), reads());
//}
//
//TEST_CASE(wait_test_non_read_write, "test that writer tasks of unrelated resources are executed before reader task is finished") {
//    return common_wait_test(writes(), reads(0), writes(1), reads(), false);
//}
//
//TEST_CASE(wait_test_read_write_multiple, "test that writer tasks are not executed before reader task is finished with multiple resources") {
//    return common_wait_test(writes(), reads(0, 1), writes(0, 1), reads()) &&
//        common_wait_test(writes(), reads(0, 1), writes(1, 0), reads()) &&
//        common_wait_test(writes(), reads(0, 1), writes(0, 2), reads()) &&
//        common_wait_test(writes(), reads(0, 1), writes(2, 0), reads());
//}
//
//TEST_CASE(wait_test_write_write, "test that writer tasks are not executed before writer task is finished") {
//    return common_wait_test(writes(0), reads(), writes(0), reads());
//}
//
//TEST_CASE(wait_test_non_write_write, "test that writer tasks of unrelated resources are executed before writer task is finished") {
//    return common_wait_test(writes(0), reads(), writes(1), reads(), false);
//}
//
//TEST_CASE(wait_test_write_write_multiple, "test that writer tasks are not executed before writer task is finished with multiple resources") {
//    return common_wait_test(writes(0, 1), reads(), writes(0, 1), reads()) &&
//        common_wait_test(writes(0, 1), reads(), writes(1, 0), reads()) &&
//        common_wait_test(writes(0, 1), reads(), writes(0, 2), reads()) &&
//        common_wait_test(writes(0, 1), reads(), writes(2, 0), reads());
//}
//
//int main() {
//    std::size_t failed = 0;
//    std::size_t total = 0;
//
//    ++total;
//    if (!wait_test_write_read()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_non_write_read()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_write_read_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_read_write()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_non_read_write()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_read_write_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_write_write()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_non_write_write()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!wait_test_write_write_multiple()) {
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
