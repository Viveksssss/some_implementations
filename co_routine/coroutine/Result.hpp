#pragma once
#include <exception>
#include <utility>

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

template <>
struct Result<void> {
    Result() { }

    Result(std::exception_ptr &&e) : _exception_ptr(e) { }

    void get_or_throw() {
        if (_exception_ptr) {
            std::rethrow_exception(_exception_ptr);
        }
    }

private:
    std::exception_ptr _exception_ptr;
};
