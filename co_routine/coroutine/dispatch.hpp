#pragma once

#include "executor.hpp"
#include <coroutine>

struct DispatchAwaiter {
    explicit DispatchAwaiter(AbstractExecutor *executor) noexcept
        : _executor(executor) { }

    bool await_ready() const {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) const {
        // 调度到协程对应的调度器上
        _executor->execute([handle]() { handle.resume(); });
    }

    void await_resume() { }

private:
    AbstractExecutor *_executor;
};
