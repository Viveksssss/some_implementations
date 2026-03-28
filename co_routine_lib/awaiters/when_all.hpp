#pragma once

#include "awaiter.hpp"
#include "Task.hpp"
#include <array>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <span>
#include <tuple>

struct WhenAllCounter {
    std::coroutine_handle<> handle{};
    std::size_t counter{};
    std::exception_ptr exception{};
};

template <typename T>
ReturnPreviousTask WhenAllHelper(
    auto &&t, WhenAllCounter &counter, Uninitialized<T> &tt) {
    try {
        if constexpr (std::is_void_v<T>) {
            co_await t;
            tt.putValue(NonVoidHelper<>{});
        } else {
            tt.putValue(co_await t);
        }
    } catch (...) {
        counter.exception = std::current_exception();
        co_return counter.handle;
    }
    --counter.counter;
    if (counter.counter == 0) {
        co_return counter.handle;
    }
    co_return std::noop_coroutine();
}

struct WhenAllAwaiter {
    WhenAllAwaiter(WhenAllCounter &counter, std::span<ReturnPreviousTask> spans)
        : tasks(spans)
        , counter(counter) { }

    bool await_ready() const noexcept {
        return false;
    }

    auto await_suspend(std::coroutine_handle<> coroutine)
        -> std::coroutine_handle<> {
        if (tasks.empty()) {
            return coroutine;
        }

        counter.handle = coroutine;
        for (auto &task: tasks.subspan(1)) {
            task.mCoroutine.resume();
        }
        return tasks.front().mCoroutine;
    }

    void await_resume() const noexcept {
        if (counter.exception) [[unlikely]] {
            std::rethrow_exception(counter.exception);
        }
    }

private:
    WhenAllCounter &counter;
    std::span<ReturnPreviousTask> tasks;
};

template <std::size_t... Is, class... Ts>
Task<std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>> whenAllImpl(
    std::index_sequence<Is...>, Ts &&...ts) {
    WhenAllCounter counter{};
    counter.counter = sizeof...(Is);
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    std::array<ReturnPreviousTask, sizeof...(Is)> tasks{
        WhenAllHelper(ts, counter, std::get<Is>(result))...};
    co_await WhenAllAwaiter(counter, tasks);
    co_return std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>(
        std::get<Is>(result).moveValue()...);
}

namespace co_async {
template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_all(Ts &&...ts) {
    return whenAllImpl(
        std::make_index_sequence<sizeof...(Ts)>{}, std::forward<Ts>(ts)...);
}
} // namespace co_async
