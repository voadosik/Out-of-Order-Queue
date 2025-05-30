//#include <cstddef>
//
//#include <iostream>
//#include <random>
//
//#include "test-common.hpp"
//
//#include "queue.hpp"
//
//namespace {
//
//    class RandomAccessTask {
//    public:
//        RandomAccessTask(Queue& queue, std::size_t write_resource, std::size_t level, std::size_t max_levels, std::mt19937 generator, std::uniform_int_distribution<resource_id> distribution)
//            : queue_(&queue), write_resource_(write_resource), level_(level), max_levels_(max_levels), generator_(generator), distribution_(distribution) {
//        }
//
//        void operator()() {
//            if (level_ >= max_levels_) {
//                return;
//            }
//
//            const auto next_write_resource = distribution_(generator_);
//
//            if (next_write_resource == write_resource_) {
//                queue_->enqueue(RandomAccessTask(*queue_, next_write_resource, level_ + 1, max_levels_, generator_, distribution_), writes(next_write_resource), reads());
//            }
//            else {
//                queue_->enqueue(RandomAccessTask(*queue_, next_write_resource, level_ + 1, max_levels_, generator_, distribution_), writes(next_write_resource), reads(write_resource_));
//            }
//        }
//
//    private:
//        Queue* queue_;
//        resource_id write_resource_;
//
//        std::size_t level_;
//        std::size_t max_levels_;
//
//        std::mt19937 generator_;
//        std::uniform_int_distribution<resource_id> distribution_;
//    };
//
//} // namespace
//
//template<std::size_t ResourceSpace, std::size_t Levels>
//static bool random_access_test() {
//    PRINT_INDENTED("Random access test with resource space " << ResourceSpace << " and " << Levels << " steps");
//
//    Queue queue;
//
//    std::mt19937 generator(42);
//    std::uniform_int_distribution<resource_id> distribution(0, ResourceSpace - 1);
//
//    const auto first_write_resource = distribution(generator);
//
//    queue.enqueue(RandomAccessTask(queue, first_write_resource, 0, Levels, generator, distribution), writes(first_write_resource), reads());
//
//    queue.serve();
//
//    return true;
//}
//
//TEST_CASE(simple_chain, "test a simple chain of tasks") {
//    return random_access_test<1, 500'000>();
//}
//
//TEST_CASE(kilo_resources, "test a kilo resources") {
//    return random_access_test<1024, 500'000>();
//}
//
//TEST_CASE(almost_mega_resources, "test almost a mega resources") {
//    return random_access_test<32 * 1024, 500'000>();
//}
//
//TEST_CASE(mega_resources, "test a mega resources") {
//    return random_access_test<1024 * 1024, 500'000>();
//}
//
//TEST_CASE(almost_giga_resources, "test almost a giga resources") {
//    return random_access_test<32 * 1024 * 1024, 500'000>();
//}
//
//TEST_CASE(giga_resources, "test a giga resources") {
//    return random_access_test<1024 * 1024 * 1024, 500'000>();
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
//    if (!kilo_resources()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!almost_mega_resources()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!mega_resources()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!almost_giga_resources()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!giga_resources()) {
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
