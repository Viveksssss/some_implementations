#pragma once

#include "Result.hpp"
#include "TaskAwaiter.hpp"
#include "dispatch.hpp"
#include <condition_variable>
#include <coroutine>
#include <functional>
#include <list>
#include <mutex>
#include <optional>

template <typename T, typename Executor>
struct TaskPromise {
    std::optional<Result<T>> result;

    DispatchAwaiter initial_suspend() {
        return DispatchAwaiter{&executor};
    }

    std::suspend_always final_suspend() noexcept {
        return {};
    }

    Task<T, Executor> get_return_object() {
        return Task<T, Executor>{
            std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }

    void unhandled_exception() noexcept {
        std::lock_guard lock(completion_lock);
        result = std::make_optional<Result<T>>(std::current_exception());
        completion.notify_all();
        notify_callbacks();
    }

    void return_value(T &&value) noexcept {
        std::lock_guard lock(completion_lock);
        result = Result<T>(std::forward<T>(value));
        completion.notify_all();
        notify_callbacks();
        // result = std::make_optional<Result<T>>(std::forward<T>(value));
    }

    template <typename U, typename Executors>
    TaskAwaiter<U, Executors> await_transform(Task<U, Executors> &&u) {
        return TaskAwaiter<U, Executors>{&executor,std::forward<Task<U, Executors>>(u)};
    }

    T get_result() {
        // blocking for result or throw on exception
        std::unique_lock lock(completion_lock);
        if (!result.has_value()) {
            completion.wait(lock);
        }
        return result->get_or_throw();
    }

    void on_completed(std::function<void(Result<T>)> &&func) {
        std::unique_lock lock(completion_lock);
        if (result.has_value()) {
            auto value = result.value();
            lock.unlock();
            func(value);
        } else {
            completion_callbacks.push_back(func);
        }
    }

private:
    void notify_callbacks() {
        auto value = result.value();
        for (auto &callback: completion_callbacks) {
            callback(value);
        }
        completion_callbacks.clear();
    }

    Executor executor;
    std::mutex completion_lock;
    std::condition_variable completion;
    std::list<std::function<void(Result<T>)>> completion_callbacks;
};
