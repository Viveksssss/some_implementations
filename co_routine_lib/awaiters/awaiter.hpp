#pragma once

#include "../utils/Scheduler.hpp"
#include "common.hpp"
#include "Promise.hpp"
#include <chrono>
#include <coroutine>

struct SleepUntilPromise;

template <class A>
concept Awaiter = requires(A a, std::coroutine_handle<> h) {
    { a.await_ready() };
    { a.await_suspend(h) };
    { a.await_resume() };
};

template <class A>
concept Awaitable = Awaiter<A> || requires(A a) {
    { a.operator co_await() } -> Awaiter;
};

template <class A>
struct AwaitableTraits;

template <Awaiter A>
struct AwaitableTraits<A> {
    using RetType = decltype(std::declval<A>().await_resume());
    using NonVoidRetType = NonVoidHelper<RetType>::Type;
};

template <class A>
    requires(!Awaiter<A> && Awaitable<A>)
struct AwaitableTraits<A>
    : AwaitableTraits<decltype(std::declval<A>().operator co_await())> { };

struct SleepAwaiter {
    SleepAwaiter(auto &loop, auto timer) : loop(loop), mTimePoint(timer) { }

    bool await_ready() const noexcept {
        return std::chrono::system_clock::now() >= mTimePoint;
    }

    void await_suspend(std::coroutine_handle<SleepUntilPromise> coroutine) {
        auto &promise = coroutine.promise();
        promise.expireTime = mTimePoint;
        loop.add_timer(promise);
    }

    void await_resume() const noexcept { }

private:
    std::chrono::system_clock::time_point mTimePoint;
    Scheduler &loop;
};

struct RepeatAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<> coroutine) const noexcept {
        if (coroutine.done()) {
            return std::noop_coroutine();
        } else {
            return coroutine;
        }
    }

    void await_resume() const noexcept { }
};

struct RepeatAwaitable {
    RepeatAwaiter operator co_await() {
        return RepeatAwaiter();
    }
};

namespace co_async {
struct EpollFileAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(
        std::coroutine_handle<EpollFilePromise> coroutine) const noexcept {
        auto &promise = coroutine.promise();
        promise.fileno = fileno;
        promise.events = events;
        loop.add_listener(promise);
    }

    void await_resume() const noexcept { }

    using ClockType = std::chrono::system_clock;
    EpollLoop &loop;
    int fileno;
    uint32_t events;
};

} // namespace co_async
