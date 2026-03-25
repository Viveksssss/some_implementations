#pragma once

#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>

template <class T = void>
struct NonVoidHelper {
    using Type = T;
};

template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;
    explicit NonVoidHelper() = default;
};

template <typename T>
struct Uninitialized {
    union {
        T value;
    };

    Uninitialized() noexcept { }

    Uninitialized(Uninitialized &&) = delete;

    ~Uninitialized() noexcept { }

    T moveValue() {
        T ret(std::move(value));
        value.~T();
        return ret;
    }

    template <class... Ts>
    void putValue(Ts &&...args) {
        new (std::addressof(value)) T(std::forward<Ts>(args)...);
    }
};

template <>
struct Uninitialized<void> {
    auto moveValue() {
        return NonVoidHelper<>{};
    }

    void putValue(NonVoidHelper<>) { }
};

template <class T>
struct Uninitialized<T const> : Uninitialized<T> { };

template <class T>
struct Uninitialized<T &> : Uninitialized<std::reference_wrapper<T>> { };

template <class T>
struct Uninitialized<T &&> : Uninitialized<T> { };

struct PreviousAwaiter {
    std::coroutine_handle<> mPrevious;

    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<> coroutine) const noexcept {
        if (mPrevious && !mPrevious.done()) {
            return mPrevious;
        } else {
            return std::noop_coroutine();
        }
    }

    void await_resume() const noexcept { }
};
