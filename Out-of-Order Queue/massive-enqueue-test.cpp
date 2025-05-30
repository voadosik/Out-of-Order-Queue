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
//static constexpr std::size_t timeout_ms = 120;
//
//template<std::size_t Resources>
//static bool massive_enqueue_test(std::size_t master_tasks, std::size_t slave_tasks) {
//    constexpr std::size_t tasks_length_ms = 1;
//    constexpr std::size_t tasks_delay_ms = 20;
//
//    Queue queue;
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    for (std::size_t i = 0; i < master_tasks; ++i) {
//        [=, &queue] <std::size_t... I>(std::index_sequence<I...>) {
//            queue.enqueue([=, &queue]() {
//                std::this_thread::sleep_until(start + std::chrono::milliseconds(tasks_length_ms));
//                for (std::size_t j = 0; j < slave_tasks; ++j) {
//                    queue.enqueue([=]() {
//                        std::this_thread::sleep_until(start + (j + 2) * std::chrono::milliseconds(tasks_length_ms));
//                        }, writes(static_cast<resource_id>(I + i * Resources + master_tasks * Resources)...), reads());
//                }
//                }, writes(static_cast<resource_id>(I + i * Resources)...), reads());
//        }(std::make_index_sequence<Resources>{});
//    }
//
//    std::vector<std::thread> threads;
//    threads.reserve(master_tasks);
//
//    for (std::size_t i = 0; i < master_tasks; ++i) {
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
//    if (elapsed_ms < tasks_length_ms * slave_tasks) {
//        PRINT_INDENTED("Tasks finished too early, expected at least " << tasks_length_ms * slave_tasks << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    if (elapsed_ms > tasks_length_ms * slave_tasks + timeout_ms) {
//        PRINT_INDENTED("Tasks finished too late, expected at most " << tasks_length_ms * slave_tasks + timeout_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(massive_enqueue_32_1_1000, "test with 32 resources and 1 master task and 1000 slave tasks") {
//    return massive_enqueue_test<32>(1, 1000);
//}
//
//TEST_CASE(massive_enqueue_32_2_1000, "test with 32 resources and 2 master tasks and 1000 slave tasks") {
//    return massive_enqueue_test<32>(2, 1000);
//}
//
//TEST_CASE(massive_enqueue_32_4_1000, "test with 32 resources and 4 master tasks and 1000 slave tasks") {
//    return massive_enqueue_test<32>(4, 1000);
//}
//
//TEST_CASE(massive_enqueue_32_8_1000, "test with 32 resources and 8 master tasks and 1000 slave tasks") {
//    return massive_enqueue_test<32>(8, 1000);
//}
//
//int main() {
//    std::size_t failed = 0;
//    std::size_t total = 0;
//
//    ++total;
//    if (!massive_enqueue_32_1_1000()) {
//        ++failed;
//    }
//    ++total;
//    if (!massive_enqueue_32_2_1000()) {
//        ++failed;
//    }
//    ++total;
//    if (!massive_enqueue_32_4_1000()) {
//        ++failed;
//    }
//    ++total;
//    if (!massive_enqueue_32_8_1000()) {
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
