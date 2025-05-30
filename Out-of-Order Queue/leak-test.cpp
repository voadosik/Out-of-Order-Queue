///**
// * This test is completely single-threaded, it should help with debugging
// *
// */
//
//#include <cstddef>
//
//#include <iostream>
//#include <utility>
//
//#include "test-common.hpp"
//
//#include "queue.hpp"
//
//namespace {
//
//    class EnqueueTask {
//    public:
//        EnqueueTask(Queue* queue, resource_id resource1, resource_id resource2, std::size_t* done_tasks, std::size_t iterations)
//            : queue_(queue), resource1_(resource1), resource2_(resource2), done_tasks_(done_tasks), iterations_(iterations) {
//        }
//
//        void operator()() {
//            using std::swap;
//            swap(resource1_, resource2_);
//
//            if (iterations_-- == 0) {
//                return;
//            }
//
//            ++ * done_tasks_;
//
//            queue_->enqueue(EnqueueTask(queue_, resource1_, resource2_, done_tasks_, iterations_), writes(resource1_), reads(resource2_));
//        }
//
//    private:
//        Queue* queue_;
//        resource_id resource1_, resource2_;
//        std::size_t* done_tasks_;
//        std::size_t iterations_;
//    };
//
//} // namespace
//
//TEST_CASE(many_queues, "just construct and destruct the queue many times") {
//    constexpr std::size_t iterations = 2'500'000;
//
//    for (std::size_t i = 0; i < iterations; ++i) {
//        Queue queue;
//        queue.serve();
//    }
//
//    return true;
//}
//
//TEST_CASE(many_jobs, "just enqueue and serve many jobs") {
//    constexpr std::size_t iterations = 2'500'000;
//    std::size_t done_tasks{ 0 };
//    Queue queue;
//
//    for (std::size_t i = 0; i < iterations; ++i) {
//        queue.enqueue([&done_tasks]() {
//            ++done_tasks;
//            }, writes(), reads());
//    }
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != iterations) {
//        PRINT_INDENTED("Tasks were not executed the expected number of times, expected " << iterations << " but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(iterate_many_times, "enqueue a task that enqueues itself many times") {
//    constexpr std::size_t iterations = 2'500'000;
//    std::size_t done_tasks{ 0 };
//    const resource_id resource1 = 0;
//    const resource_id resource2 = 1;
//
//    Queue queue;
//
//    queue.enqueue(EnqueueTask(&queue, resource1, resource2, &done_tasks, iterations), writes(resource1), reads(resource2));
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != iterations) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected " << iterations << " but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(iterate_many_times_multiple, "enqueue a task that enqueues itself many times") {
//    constexpr std::size_t iterations = 250'000;
//    constexpr std::size_t tasks = 10;
//    std::size_t done_tasks{ 0 };
//    const resource_id resource1 = 0;
//    const resource_id resource2 = 1;
//
//    Queue queue;
//
//    for (std::size_t i = 0; i < tasks; ++i) {
//        queue.enqueue(EnqueueTask(&queue, resource1, resource2, &done_tasks, iterations), writes(resource1), reads(resource2));
//    }
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != iterations * tasks) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected " << iterations * tasks << " but got " << done_tasks);
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
//    if (!many_queues()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!many_jobs()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!iterate_many_times()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!iterate_many_times_multiple()) {
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
