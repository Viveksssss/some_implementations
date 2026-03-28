#pragma once

#include "Promise.hpp"
#include <coroutine>

template <typename T = void, class P = Promise<T>>
struct [[nodiscard]] Task {
    using promise_type = P;

    Task(std::coroutine_handle<promise_type> coroutine)
        : mCoroutine(coroutine) { }

    std::coroutine_handle<promise_type> mCoroutine;

    struct Awaiter {
        bool await_ready() const {
            return false;
        }

        std::coroutine_handle<> await_suspend(
            std::coroutine_handle<> coroutine) const {
            mCoroutine.promise().mPrevious = coroutine;
            return mCoroutine;
        }

        auto await_resume() const {
            return mCoroutine.promise().result();
        }

        std::coroutine_handle<promise_type> mCoroutine;
    };

    auto operator co_await() const {
        return Awaiter(mCoroutine);
    }

    operator std::coroutine_handle<>() const noexcept {
        return mCoroutine;
    }

    ~Task() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }
};

struct ReturnPreviousTask {
    using promise_type = ReturnPreviousPromise;

    ReturnPreviousTask(std::coroutine_handle<promise_type> coroutine) noexcept
        : mCoroutine(coroutine) { }

    ReturnPreviousTask(ReturnPreviousTask &&other) noexcept
        : mCoroutine(other.mCoroutine) {
        other.mCoroutine = nullptr;
    }

    ~ReturnPreviousTask() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }

    std::coroutine_handle<> mCoroutine;
};

namespace co_async {
template <typename Loop, typename T, typename P>
void run_task(Loop &loop, Task<T, P> const &t) {
    t.mCoroutine.resume();
    while (loop.run())
        ;
}
}; // namespace co_async
