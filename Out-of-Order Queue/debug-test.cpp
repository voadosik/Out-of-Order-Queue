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
//TEST_CASE(construct_destruct, "just construct and destruct the queue") {
//    Queue queue;
//
//    return true;
//}
//
//TEST_CASE(no_work, "serve() the queue without any tasks, the thread should exit") {
//    Queue queue;
//
//    queue.serve();
//
//    return true;
//}
//
//TEST_CASE(task_done, "Just do one task with no dependencies") {
//    std::size_t done_tasks{ 0 };
//
//    Queue queue;
//
//    queue.enqueue([&done_tasks]() {
//        ++done_tasks;
//        }, writes(), reads());
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_reads, "Execute a task that reads a resource") {
//    std::size_t done_tasks{ 0 };
//    const resource_id resource = 0;
//
//    Queue queue;
//
//    queue.enqueue([&done_tasks]() {
//        ++done_tasks;
//        }, writes(), reads(resource));
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_reads_not, "Execute a task that reads nothing") {
//    std::size_t done_tasks{ 0 };
//
//    Queue queue;
//
//    queue.enqueue([&done_tasks]() {
//        ++done_tasks;
//        }, writes(), reads());
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_reads_multiple, "Execute a task that reads multiple resources") {
//    constexpr std::size_t resources = 32;
//    std::size_t done_tasks{ 0 };
//
//    Queue queue;
//
//    [&queue, &done_tasks] <std::size_t... I>(std::index_sequence<I...>) {
//        queue.enqueue([&done_tasks]() {
//            ++done_tasks;
//            }, writes(), reads(static_cast<resource_id>(I)...));
//    }(std::make_index_sequence<resources>{});
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(multiple_tasks_that_reads, "Execute multiple tasks that read a resource") {
//    std::size_t done_tasks{ 0 };
//    constexpr std::size_t expected_tasks{ 10 };
//    const resource_id resource = 0;
//
//    Queue queue;
//
//    for (std::size_t i = 0; i < expected_tasks; ++i) {
//        queue.enqueue([&done_tasks]() {
//            ++done_tasks;
//            }, writes(), reads(resource));
//    }
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != expected_tasks) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected " << expected_tasks << " but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_writes, "Execute a task that writes a resource") {
//    std::size_t done_tasks{ 0 };
//    const resource_id resource = 0;
//    Queue queue;
//
//    queue.enqueue([&done_tasks]() {
//        ++done_tasks;
//        }, writes(resource), reads());
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_writes_not, "Execute a task that writes nothing") {
//    std::size_t done_tasks{ 0 };
//    Queue queue;
//
//    queue.enqueue([&done_tasks]() {
//        ++done_tasks;
//        }, writes(), reads());
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_writes_multiple, "Execute a task that writes multiple resources") {
//    constexpr std::size_t resources = 32;
//    std::size_t done_tasks{ 0 };
//    Queue queue;
//
//    [&queue, &done_tasks] <std::size_t... I>(std::index_sequence<I...>) {
//        queue.enqueue([&done_tasks]() {
//            ++done_tasks;
//            }, writes(static_cast<resource_id>(I)...), reads());
//    }(std::make_index_sequence<resources>{});
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(multiple_tasks_that_writes, "Execute multiple tasks that write a resource") {
//    std::size_t done_tasks{ 0 };
//    constexpr std::size_t expected_tasks{ 10 };
//    const resource_id resource = 0;
//
//    Queue queue;
//
//    for (std::size_t i = 0; i < expected_tasks; ++i) {
//        queue.enqueue([&done_tasks, i]() {
//            if (done_tasks != i) {
//                PRINT_INDENTED("Task was not executed in the expected order, expected " << done_tasks << " but got " << i);
//            }
//
//            ++done_tasks;
//            }, writes(resource), reads());
//    }
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != expected_tasks) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected " << expected_tasks << " but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_writes_and_reads, "Execute a task that writes and reads a resource") {
//    std::size_t done_tasks{ 0 };
//    const resource_id resource1 = 0;
//    const resource_id resource2 = 1;
//
//    Queue queue;
//
//    queue.enqueue([&done_tasks]() {
//        ++done_tasks;
//        }, writes(resource1), reads(resource2));
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
//        return false;
//    }
//
//    return true;
//}
//
//TEST_CASE(task_that_writes_and_reads_multiple, "Execute a task that writes and reads multiple resources") {
//    constexpr std::size_t resources = 32;
//
//    std::size_t done_tasks{ 0 };
//
//    Queue queue;
//
//    [&queue, &done_tasks] <std::size_t... I>(std::index_sequence<I...>) {
//        queue.enqueue([&done_tasks]() {
//            ++done_tasks;
//            }, writes(static_cast<resource_id>(I)...), reads(static_cast<resource_id>(resources + I)...));
//    }(std::make_index_sequence<resources>{});
//
//    if (done_tasks != 0) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 0 but got " << done_tasks);
//        return false;
//    }
//
//    queue.serve();
//
//    if (done_tasks != 1) {
//        PRINT_INDENTED("Task was not executed the expected number of times, expected 1 but got " << done_tasks);
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
//
//    ++total;
//    if (!task_done()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_reads()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_reads_not()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_reads_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!multiple_tasks_that_reads()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes_not()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!task_that_writes_multiple()) {
//        ++failed;
//    }
//
//    ++total;
//    if (!multiple_tasks_that_writes()) {
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
