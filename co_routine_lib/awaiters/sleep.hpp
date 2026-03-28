#pragma once

#include "awaiter.hpp"
#include "Task.hpp"

namespace co_async {

inline Task<void, SleepUntilPromise> sleep_for(
    std::chrono::system_clock::duration duration) {
    auto &loop = co_async::getTimerLoop();
    co_await SleepAwaiter(loop, std::chrono::system_clock::now() + duration);
}

inline Task<void, SleepUntilPromise> sleep_until(
    std::chrono::system_clock::time_point time_point) {
    auto &loop = co_async::getTimerLoop();
    co_await SleepAwaiter(loop, time_point);
}

} // namespace co_async
