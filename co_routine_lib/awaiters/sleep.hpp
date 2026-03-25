#pragma once

#include "awaiter.hpp"
#include "Task.hpp"

inline Task<void, SleepUntilPromise> sleep_for(
    std::chrono::system_clock::duration duration) {
    auto &loop = getLoop();
    co_await SleepAwaiter(loop, std::chrono::system_clock::now() + duration);
}

inline Task<void, SleepUntilPromise> sleep_until(
    std::chrono::system_clock::time_point time_point) {
    auto &loop = getLoop();
    co_await SleepAwaiter(loop, time_point);
}
