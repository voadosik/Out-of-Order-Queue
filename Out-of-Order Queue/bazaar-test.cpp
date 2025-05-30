//#include <cassert>
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
//namespace {
//
//    template<std::size_t Resources> requires (Resources >= 1)
//        class BazaarTask {
//        public:
//            BazaarTask(Queue& queue, std::size_t task_id, std::size_t level, std::size_t max_levels, 
//                std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now(), std::size_t delay_ms = 0)
//                : queue_(&queue), task_id_(task_id), level_(level), max_levels_(max_levels), start_(start), delay_ms_(delay_ms) {
//            }
//
//            void operator()() {
//                std::this_thread::sleep_until(start_);
//
//                if (level_ >= max_levels_) {
//                    return;
//                }
//
//                const auto next_start = start_ + std::chrono::milliseconds(delay_ms_);
//                const auto next_task_id = task_id_ + Resources;
//
//                const auto written_resource = static_cast<resource_id>(next_task_id);
//                const auto first_read_resource = static_cast<resource_id>(task_id_ / Resources * Resources);
//
//                assert(level_ < max_levels_);
//
//                [=, this] <std::size_t... I>(std::index_sequence<I...>) {
//                    queue_->enqueue(BazaarTask(*queue_, next_task_id, level_ + 1, max_levels_, next_start, delay_ms_), writes(written_resource), reads(static_cast<resource_id>(first_read_resource + (task_id_ + I) % Resources)...));
//                }(std::make_index_sequence<Resources>{});
//            }
//
//        private:
//            Queue* queue_;
//            std::size_t task_id_;
//            std::size_t level_;
//            std::size_t max_levels_;
//
//            std::chrono::steady_clock::time_point start_;
//            std::size_t delay_ms_;
//    };
//
//} // namespace
//
//template<std::size_t Resources, std::size_t Levels>
//static bool bazaar_test() {
//    constexpr std::size_t tasks_delay_ms = Resources == 1 ? 1 : 20;
//
//    PRINT_INDENTED("Bazaar test with " << Resources << " resources and " << Levels << " steps");
//
//    Queue queue;
//
//    const auto start = std::chrono::steady_clock::now() + std::chrono::milliseconds(tasks_delay_ms);
//
//    // create Bazaar
//    for (std::size_t i = 0; i < Resources; ++i) {
//        queue.enqueue(BazaarTask<Resources>(queue, i, 0, Levels, start, tasks_delay_ms), writes(i), reads());
//    }
//
//    std::vector<std::thread> threads;
//    threads.reserve(Resources);
//
//    for (std::size_t i = 0; i < Resources; ++i) {
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
//    if (elapsed_ms < Levels * tasks_delay_ms) {
//        PRINT_INDENTED("Tasks finished too early, expected at least " << Levels * tasks_delay_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    if (elapsed_ms > Levels * tasks_delay_ms + timeout_ms) {
//        PRINT_INDENTED("Tasks finished too late, expected at most " << Levels * tasks_delay_ms + timeout_ms << "ms but got " << elapsed_ms);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(simple_chain, "test a simple chain of tasks") {
//    return bazaar_test<1, 2000>();
//}
//
//TEST_CASE(bazaar_128, "test with 128 resources and 16 steps") {
//    return bazaar_test<128, 16>();
//}
//
//TEST_CASE(bazaar_64, "test with 64 resources and 32 steps") {
//    return bazaar_test<64, 8>();
//}
//
//TEST_CASE(bazaar_32, "test with 32 resources and 64 steps") {
//    return bazaar_test<32, 64>();
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
//    if (!bazaar_128()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!bazaar_64()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!bazaar_32()) {
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
