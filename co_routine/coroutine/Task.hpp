#include "executor.hpp"
#include "TaskPromise.hpp"
#include <coroutine>
#include <exception>
#include <functional>

// template<typename T>
template <typename T, typename Executor = NewThreadExecutor>
struct Task {
    using promise_type = TaskPromise<T, Executor>;

    T get_result() {
        return handle.promise().get_result();
    }

    Task &then(std::function<void(T)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                func(result.get_or_throw());
            } catch (std::exception &e) {}
        });
        return *this;
    }

    Task &catching(std::function<void(std::exception &)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                result.get_or_throw();
            } catch (std::exception &e) {
                func(e);
            }
        });
        return *this;
    }

    Task &finally(std::function<void()> &&func) {
        handle.promise().on_completed([func](auto result) { func(); });
        return *this;
    }

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept
        : handle(handle) { };

    Task(Task &&other) noexcept
        : handle(std::exchange(other.handle, nullptr)) { }

    Task(Task const &) = delete;
    Task &operator=(Task const &) = delete;

    ~Task() {
        if (handle) {
            handle.destroy();
        }
    }

    std::coroutine_handle<promise_type> handle;
};
