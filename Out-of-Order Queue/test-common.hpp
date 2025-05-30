#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <cstddef>

#include <atomic>
#include <array>
#include <chrono>
#include <iostream>
#include <ranges>
#include <string>
#include <thread>

#include "queue.hpp"

static std::size_t indent = 0;

constexpr std::size_t INDENT_STEP = 2;

inline void wait_for_time_or_done(std::atomic<bool>& done, std::size_t timeout_ms, std::size_t sleep_ms = 10) {
    auto start = std::chrono::steady_clock::now();

    while (!done.load() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(timeout_ms)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
}

#define PRINT_INDENTED(...) \
    do { \
        std::cout << std::string(indent, ' ') __VA_OPT__(<<) __VA_ARGS__ << std::endl; \
    } while (0)

#define INFO_INDENTED(...) \
    do { \
        std::cerr << std::string(indent, ' ') __VA_OPT__(<<) __VA_ARGS__ << std::endl; \
    } while (0)

#define PUSH_INDENT() do { indent += INDENT_STEP; } while (0)
#define POP_INDENT() do { indent -= INDENT_STEP; } while (0)
#define RESET_INDENT() do { indent = 0; } while (0)

// defines a test case, has to be followed by a function body
#define TEST_CASE(name, ...) \
    static bool name ## _inner(); \
    static bool name() { \
        RESET_INDENT(); \
        PRINT_INDENTED("Test case \"" #name "\"" __VA_OPT__(<< " (" <<) __VA_ARGS__ __VA_OPT__(<< ")") << ':'); \
        PUSH_INDENT(); \
        try { \
            if (!name ## _inner()) { \
                PRINT_INDENTED("Test case \"" #name "\" failed"); \
                return false; \
            } \
            PRINT_INDENTED("Test case \"" #name "\" passed"); \
            return true; \
        } catch (const std::exception &e) { \
            PRINT_INDENTED("Test case \"" #name "\" ended with an unexpected exception: " << e.what()); \
            return false; \
        } catch (...) { \
            PRINT_INDENTED("Test case \"" #name "\" ended with an unknown exception\n"); \
            return false; \
        } \
    } \
    static bool name ## _inner()

template<class... Args>
inline auto writes(Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return std::ranges::empty_view<resource_id>();
    }
    else if constexpr (sizeof...(args) == 1) {
        return std::ranges::single_view<resource_id>(args...);
    }
    else {
        return std::array<resource_id, sizeof...(args)>{static_cast<resource_id>(args)...};
    }
}

template<class... Args>
inline auto reads(Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return std::ranges::empty_view<resource_id>();
    }
    else if constexpr (sizeof...(args) == 1) {
        return std::ranges::single_view<resource_id>(args...);
    }
    else {
        return std::array<resource_id, sizeof...(args)>{static_cast<resource_id>(args)...};
    }
}

#endif // TEST_COMMON_HPP
