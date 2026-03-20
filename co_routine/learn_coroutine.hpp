#pragma once

#include <chrono>
#include <coroutine>
#include <future>
#include <iostream>

struct Task {
    struct promise_type {
        int value;

        Task get_return_object() noexcept {
            return Task{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void return_void() noexcept { }

        void unhandled_exception() noexcept { }

        int get_value() noexcept {
            return value;
        }

        void set_value(int v) noexcept {
            value = v;
        }
    };

    std::coroutine_handle<promise_type> handle;

    auto get_handle() noexcept {
        return handle;
    }

    Task(std::coroutine_handle<promise_type> h) : handle(h) { }

    ~Task() {
        if (handle && handle.done()) {
            // 只有完成的协程才自动销毁
            handle.destroy();
        }
    }
};

struct await {
    int value;

    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> awaiter) noexcept {
        std::thread([=]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            awaiter.resume();
        }).detach();
    }

    int await_resume() noexcept {
        std::cout << "await_resume called\n"; // 这个永远不会输出！
        return value * 10;
    }

    await(int v) : value(v) { }
};
