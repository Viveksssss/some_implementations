#pragma once

#include "awaiter.hpp"
#include "Task.hpp"
#include <array>
#include <coroutine>
#include <cstddef>
#include <span>
#include <tuple>
#include <variant>

struct WhenAnyCounter {
    static constexpr std::size_t kNullIndex = std::size_t(-1);
    std::coroutine_handle<> handle{};
    std::size_t index{kNullIndex};
    std::exception_ptr exception{};
};

template <typename T>
ReturnPreviousTask WhenAnyHelper(Task<T> const &t, WhenAnyCounter &counter,
    Uninitialized<T> &tt, std::size_t index) {
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

    counter.index = index;
    co_return counter.handle;
}

struct WhenAnyAwaiter {
    WhenAnyAwaiter(WhenAnyCounter &counter, std::span<ReturnPreviousTask> spans)
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
    WhenAnyCounter &counter;
    std::span<ReturnPreviousTask> tasks;
};

template <std::size_t... Is, class... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::NonVoidRetType...>> whenAnyImpl(
    std::index_sequence<Is...>, Ts &&...ts) {
    WhenAnyCounter counter{};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    std::array<ReturnPreviousTask, sizeof...(Is)> tasks{
        WhenAnyHelper(ts, counter, std::get<Is>(result), Is)...};
    co_await WhenAnyAwaiter(counter, tasks);

    // 直接构造 variant，不使用 Uninitialized 包装
    std::variant<typename AwaitableTraits<Ts>::NonVoidRetType...> var;
    // 使用逗号表达式和折叠表达式来设置正确的值
    ((counter.index == Is && (var = std::get<Is>(result).moveValue(), true)),
        ...);

    co_return var;
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_any(Ts... ts) {
    return whenAnyImpl(
        std::make_index_sequence<sizeof...(Ts)>{}, std::forward<Ts>(ts)...);
}
