#pragma once

#include "../utils/rbtree.hpp"
#include "common.hpp"
#include <chrono>
#include <coroutine>
#include <exception>

template <typename T>
struct Promise {
    auto initial_suspend() {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter{mPrevious};
    }

    void unhandled_exception() {
        exception = std::current_exception();
    }

    void return_value(T v) {
        mResult.putValue(v);
    }

    auto result() {
        if (exception) [[unlikely]] {
            std::rethrow_exception(exception);
        }
        return mResult.moveValue();
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    Uninitialized<T> mResult;
    std::coroutine_handle<> mPrevious;
    std::exception_ptr exception;
};

template <>
struct Promise<void> {
    auto initial_suspend() {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter{mPrevious};
    }

    void unhandled_exception() {
        exception = std::current_exception();
    }

    void return_void() { }

    auto result() {
        if (exception) [[unlikely]] {
            std::rethrow_exception(exception);
        }
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::exception_ptr exception;
    std::coroutine_handle<> mPrevious;
};

struct ReturnPreviousPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() {
        throw;
    }

    void return_value(std::coroutine_handle<> previous) noexcept {
        mPrevious = previous;
    }

    auto get_return_object() {
        return std::coroutine_handle<ReturnPreviousPromise>::from_promise(
            *this);
    }

    std::coroutine_handle<> mPrevious{};

    ReturnPreviousPromise &operator=(ReturnPreviousPromise &&) = delete;
};

struct SleepUntilPromise : RbTree<SleepUntilPromise>::RbNode, Promise<void> {
    auto get_return_object() {
        return std::coroutine_handle<SleepUntilPromise>::from_promise(*this);
    }

    SleepUntilPromise &operator=(SleepUntilPromise &&) = delete;

    friend bool operator<(
        SleepUntilPromise const &lhs, SleepUntilPromise const &rhs) noexcept {
        return lhs.expireTime < rhs.expireTime;
    };

    std::chrono::system_clock::time_point expireTime;
};



namespace co_async{

    struct EpollFilePromise : Promise<void> {
        auto get_return_object() {
            return std::coroutine_handle<EpollFilePromise>::from_promise(*this);
        }
        
        EpollFilePromise &operator=(EpollFilePromise &&) = delete;
        
        int fileno;
        uint32_t events;
    };
    
}