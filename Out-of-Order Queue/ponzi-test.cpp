//#include <cstddef>
//
//#include <chrono>
//#include <iostream>
//#include <vector>
//#include <thread>
//
//#include "test-common.hpp"
//
//#include "queue.hpp"
//
//static constexpr std::size_t timeout_ms = 120;
//
//namespace {
//
//    constexpr std::size_t exp(std::size_t base, std::size_t exponent) {
//        if (exponent == 0) {
//            return 1;
//        }
//
//        if (exponent == 1) {
//            return base;
//        }
//
//        std::size_t half = exponent / 2;
//        std::size_t base_to_half = exp(base, half);
//
//        if (exponent % 2 == 0) {
//            return base_to_half * base_to_half;
//        }
//        else {
//            return base_to_half * base_to_half * base;
//        }
//    }
//
//    template<std::size_t PonziMultiplier>
//    class PonziTask {
//    public:
//        PonziTask(Queue& queue, std::size_t task_id, std::size_t level, std::size_t max_levels, std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now(), std::size_t delay_ms = 0)
//            : queue_(&queue), task_id_(task_id), level_(level), max_levels_(max_levels), start_(start), delay_ms_(delay_ms) {
//        }
//
//        void operator()() {
//            std::this_thread::sleep_until(start_);
//
//            if (level_ < max_levels_) {
//                for (std::size_t i = 0; i < PonziMultiplier; ++i) {
//                    const auto task_id = exp(PonziMultiplier, level_) + task_id_ * PonziMultiplier + i;
//                    const auto resource = static_cast<resource_id>(task_id_);
//                    const auto next_resource = static_cast<resource_id>(task_id);
//                    const auto next_start = start_ + std::chrono::milliseconds(delay_ms_);
//                    queue_->enqueue(PonziTask(*queue_, task_id, level_ + 1, max_levels_, next_start, delay_ms_), writes(next_resource), reads(resource));
//                }
//            }
//        }
//
//    private:
//        Queue* queue_;
//        std::size_t task_id_;
//        std::size_t level_;
//        std::size_t max_levels_;
//
//        std::chrono::steady_clock::time_point start_;
//        std::size_t delay_ms_;
//    };
//
//} // namespace
//
//template<std::size_t PonziMultiplier>
//static bool ponzi_test() {
//    constexpr std::size_t target_tasks = 2000;
//
//    std::size_t levels;
//    for (levels = 0; levels < target_tasks; ++levels) {
//        if (exp(PonziMultiplier, levels) > target_tasks) {
//            break;
//        }
//    }
//
//
//    const std::size_t workers = exp(PonziMultiplier, levels - 1);
//    const std::size_t tasks_delay_ms = workers == 1 ? 1 : 20;
//
//    PRINT_INDENTED("Ponzi test with multiplier " << PonziMultiplier << " and " << levels << " levels, total workers: " << workers);
//
//    Queue queue;
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    // create Ponzi
//    queue.enqueue(PonziTask<PonziMultiplier>(queue, 0, 0, levels, start, tasks_delay_ms), writes(0), reads());
//
//    std::vector<std::thread> threads;
//    threads.reserve(workers);
//
//    for (std::size_t i = 0; i < workers; ++i) {
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
//    if (elapsed_ms < levels * tasks_delay_ms) {
//        PRINT_INDENTED("Tasks finished too early, expected at least " << levels * tasks_delay_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    if (elapsed_ms > levels * tasks_delay_ms + timeout_ms) {
//        PRINT_INDENTED("Tasks finished too late, expected at most " << levels * tasks_delay_ms + timeout_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(simple_chain, "test a simple chain of tasks") {
//    return ponzi_test<1>();
//}
//
//TEST_CASE(binary_tree, "test a binary tree of tasks") {
//    return ponzi_test<2>();
//}
//
//TEST_CASE(ternary_tree, "test a ternary tree of tasks") {
//    return ponzi_test<3>();
//}
//
//TEST_CASE(quad_tree, "test a quad tree of tasks") {
//    return ponzi_test<4>();
//}
//
//int main() {
//    std::size_t failed = 0;
//    std::size_t total = 0;
//
//    ++total;
//    if (!simple_chain()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!binary_tree()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!ternary_tree()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!quad_tree()) {
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
