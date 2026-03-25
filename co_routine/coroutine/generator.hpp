#pragma once

#include <algorithm>
#include <coroutine>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <utility>

template <typename T, typename = void>
struct is_range : std::false_type { };

template <typename T>
struct is_range<T, std::void_t<decltype(std::begin(std::declval<T &>())),
                       decltype(std::end(std::declval<T &>()))>>
    : std::true_type { };

template <typename T>
inline constexpr bool is_range_v = is_range<T>::value;

template <typename T>
struct Generator {
    struct promise_type {
        T value;
        bool has_value = false;
        std::coroutine_handle<> caller;
        std::exception_ptr e;

        Generator get_return_object() noexcept {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            if (caller && !caller.done()) {
                caller.resume();
            }
            return {};
        }

        std::suspend_always yield_value(T value) noexcept {
            this->value = value;
            has_value = true;
            return {};
        }

        std::suspend_always return_value(T value) noexcept {
            this->value = value;
            return {};
        }

        T get_value() const noexcept {
            return value;
        }

        template <typename Ts>
        auto await_transform(Ts &&gen) {
            struct Awaitable {
                std::coroutine_handle<promise_type> g;

                bool await_ready() {
                    return g.done();
                }

                void await_suspend(std::coroutine_handle<> h) {
                    g.promise().caller = h;
                    g.resume();
                    std::cout << "h:" << &h << std::endl;
                }

                T await_resume() {
                    return g.promise().get_value();
                }
            };

            return Awaitable{gen.handle};
        }

        void unhandled_exception() noexcept { 
            e = std::current_exception();
        }

        void set_value(T v) noexcept {
            value = v;
        }
    };

    int get_value() const noexcept {
        return handle.promise().value;
    }

    Generator<int> operator co_await() {
        return handle;
    }

    Generator(Generator const &) = delete;
    Generator &operator=(Generator const &) = delete;
    Generator(Generator &&other) noexcept
        : handle(std::exchange(other.handle, {})) { };

    Generator &operator=(Generator &&other) noexcept {
        handle = std::exchange(other.handle, {});
        return *this;
    }

    std::coroutine_handle<promise_type> handle;

    std::coroutine_handle<promise_type> get_handle() noexcept {
        return handle;
    }

    Generator(std::coroutine_handle<promise_type> h) : handle(h) { }

    bool has_handle = true;

    ~Generator() {
        if (handle && handle.done()) {
            std::cout << "destory" << std::endl;
            handle.destroy();
        }
    }

    bool has_next() noexcept {
        if (handle.done()) {
            return false;
        }

        if (!handle.promise().has_value) {
            handle.resume();
        }
        return !handle.done();
    }

    T next() const noexcept {
        handle.promise().has_value = false;
        return handle.promise().value;
    }

    template <typename range, typename = std::enable_if_t<is_range_v<range>>>
        requires std::ranges::range<range>
    static Generator from_array(range r) {
        for (auto &e: r) {
            co_yield e;
        }
    }

    static Generator from_array(std::initializer_list<T> r) {
        for (auto &e: r) {
            co_yield e;
        }
    }

    template <typename... Args>
    static Generator from_array(Args &&...args) {
        (co_yield args, ...);
    }

    template <typename F>
    Generator<std::invoke_result_t<F, T>> map(F &&f) {
        // auto up_stream = std::move(*this);
        while (has_next()) {
            co_yield f(next());
        }
    }

    template <typename F>
    std::invoke_result_t<F, T> flat_map(F &&f) {
        while (has_next()) {
            auto generator = f(next());
            while (generator.has_next()) {
                co_yield generator.next();
            }
        }
    }

    template <typename F>
    void for_each(F &&f) {
        while (has_next()) {
            f(next());
        }
    }

    template <typename R, typename F>
    R fold(R initial, F &&f) {
        R cc = initial;
        while (has_next()) {
            cc = f(cc, next());
        }
        return cc;
    }

    T sum() {
        T sum = 0;
        while (has_next()) {
            sum += next();
        }
        return sum;
    }

    template <typename F>
    Generator filter(F &&f) {
        while (has_next()) {
            T value = next();
            if (f(value)) {
                co_yield value;
            }
        }
    }

    Generator take(int n) {
        int i = 0;
        while (has_next() && i < n) {
            co_yield next();
            i++;
        }
    }

    template <typename Func>
    Generator take_while(Func &&f) {
        while (has_next() && f(next())) {
            co_yield next();
        }
    }
};

template <typename T>
struct Result {
    Result(T &&value) : _value(std::forward<T>(value)) { }

    Result(std::exception_ptr &&e) : _exception_ptr(e) { }

    T get_or_throw() {
        if (_exception_ptr) {
            std::rethrow_exception(_exception_ptr);
        }
        return _value;
    }

private:
    T _value;
    std::exception_ptr _exception_ptr;
};

// template<typename T>
template <typename T>
struct Task {
    struct promise_type {
        std::optional<Result<T>> result;
        template <typename R>
        struct TaskAwaiter {
            explicit TaskAwaiter(Task<R> &&task) noexcept
                : task(std::move(task)) { }

            TaskAwaiter(TaskAwaiter &&completion) noexcept
                : task(std::exchange(completion.task, {})) { }

            TaskAwaiter(TaskAwaiter &) = delete;

            TaskAwaiter &operator=(TaskAwaiter &) = delete;

            constexpr bool await_ready() const noexcept {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle) noexcept {
                // 当 task 执行完之后调用 resume
                task.finally([handle]() { handle.resume(); });
            }

            // 协程恢复执行时，被等待的 Task 已经执行完，调用 get_result
            // 来获取结果
            R await_resume() noexcept {
                return task.get_result();
            }

        private:
            Task<R> task;
        };

        std::suspend_never initial_suspend() {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        Task<T> get_return_object() {
            return Task<T>{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        void unhandled_exception() noexcept {
            result = std::make_optional<Result<T>>(std::current_exception());
        }

        void return_value(T &&value) noexcept {
            result = std::make_optional<Result<T>>(std::forward<T>(value));
        }

        template <typename U>
        TaskAwaiter<U> await_transform(U &&u) {
            return TaskAwaiter<U>{std::forward<U>(u)};
        }
    };

    std::coroutine_handle<promise_type> handle;
};
