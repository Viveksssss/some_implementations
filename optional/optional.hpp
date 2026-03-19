#pragma once

#include <exception>
#include <initializer_list>
#include <type_traits>

struct BadOptionalAccess : std::exception {
    BadOptionalAccess() = default;
    virtual ~BadOptionalAccess() = default;

    char const *what() const noexcept override {
        return "BadOptionalAccess";
    }
};

struct nullopt_t {
    explicit nullopt_t() = default;
};

inline constexpr nullopt_t nullopt;

struct inplace_t {
    explicit inplace_t() = default;
};

inline constexpr inplace_t inplace;

template <typename T>
struct optional {
private:
    bool _has_value;

    union {
        T _value;
    };

public:
    optional(T &&value) noexcept : _has_value(true) {
        new (&_value) T(std::move(value));
    }

    optional(T const &value) noexcept : _has_value(true) {
        new (&_value) T(value);
    }

    optional() noexcept : optional(nullopt) {}

    optional(nullopt_t) noexcept : _has_value(false) {}

    template <typename... Args>
    explicit optional(inplace_t, Args &&...args) : _has_value(true) {
        new (&_value) T(std::forward<Args>(args)...);
    }

    template <typename U, class... Args>
    explicit optional(inplace_t, std::initializer_list<U> list, Args &&...args)
        : _has_value(true) {
        new (&_value) T(list, std::forward<Args>(args)...);
    }

    optional(optional const &other) : _has_value(other._has_value) {
        if (_has_value) {
            new (&_value) T(other._value);
        }
    }

    optional(optional &&other) noexcept : _has_value(other._has_value) {
        if (_has_value) {
            new (&_value) T(std::move(other._value));
        }
    }

    optional &operator=(nullopt_t) noexcept {
        if (_has_value) {
            _value.~T();
            _has_value = false;
        }
        return *this;
    }

    optional &operator=(T &&value) noexcept {
        if (_has_value) {
            _value = std::move(value);
        } else {
            new (&_value) T(std::move(value));
            _has_value = true;
        }
        return *this;
    }

    optional &operator=(T const &_value) noexcept {
        if (_has_value) {
            _value.~T();
            _has_value = false;
        }
        new (&_value) T(_value);
        _has_value = true;
        return *this;
    }

    optional &operator=(optional const &other) {
        if (this == &other) {
            return *this;
        }

        if (_has_value) {
            _value.~T();
            _has_value = false;
        }
        if (other._has_value) {
            new (&_value) T(other._value);
        }
        _has_value = other._has_value;
        return *this;
    }

    optional &operator=(optional &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (_has_value && other._has_value) {
            _value = std::move(other._has_value);
            _has_value = true;
        } else if (_has_value) {
            _value.~T();
            _has_value = false;
        } else if (other._has_value) {
            new (&_value) T(std::move(other._value));
            _has_value = true;
        }
        other._has_value = false;
        return *this;
    }

    template <class... Args>
    void emplace(Args &&...args) {
        if (_has_value) {
            _value.~T();
            _has_value = false;
        }
        new (&_value) T(std::forward<Args>(args)...);
        _has_value = true;
    }

    template <class U, class... Args>
    void emplace(std::initializer_list<U> list, Args &&...args) {
        if (_has_value) {
            _value.~T();
            _has_value = false;
        }
        new (&_value) T(list, std::forward<Args>(args)...);
        _has_value = true;
    }

    void reset() noexcept {
        if (_has_value) {
            _value.~T();
            _has_value = false;
        }
    }

    ~optional() {
        if (_has_value) {
            _value.~T();
        }
    }

    bool has_value() const noexcept {
        return _has_value;
    }

    explicit operator bool() noexcept {
        return _has_value;
    }

    bool operator==(nullopt_t) noexcept {
        return !_has_value;
    }

    friend bool operator==(nullopt_t, optional const &self) noexcept {
        return !self._has_value;
    }

    bool operator==(optional const &other) noexcept {
        if (_has_value != other._has_value) {
            return false;
        }
        return _value == other._value;
    }

    bool operator!=(nullopt_t) const noexcept {
        return _has_value;
    }

    friend bool operator!=(nullopt_t, optional const &self) noexcept {
        return self._has_value;
    }

    bool operator!=(optional const &other) noexcept {
        if (_has_value == other._has_value) {
            return false;
        }
        return _value != other._value;
    }

    T const &value() const & {
        if (!_has_value) {
            throw BadOptionalAccess();
        }
        return _value;
    }

    T &value() & {
        if (!_has_value) {
            throw BadOptionalAccess();
        }
        return _value;
    }

    T const &&value() const && {
        if (!_has_value) {
            throw BadOptionalAccess();
        }
        return std::move(_value);
    }

    T &&value() && {
        if (!_has_value) {
            throw BadOptionalAccess();
        }
        return std::move(_value);
    }

    T const &operator*() const & noexcept {
        return _value;
    }

    T &operator*() & noexcept {
        return _value;
    }

    T const &&operator*() const && noexcept {
        return std::move(_value);
    }

    T &&operator*() && noexcept {
        return std::move(_value);
    }

    T const *operator->() const noexcept {
        return &_value;
    }

    T *operator->() noexcept {
        return &_value;
    }

    T value_or(T &&default_value) const & {
        return _has_value ? _value : std::move(default_value);
    }

    T value_or(T const &default_value) const & {
        return _has_value ? _value : default_value;
    }

    bool operator==(optional<T> const &other) const noexcept {
        if (_has_value != other._has_value) {
            return false;
        }
        if (_has_value) {
            return _value == other._value;
        }
        return true;
    }

    bool operator!=(optional const &other) const noexcept {
        if (_has_value != other._has_value) {
            return true;
        }
        if (_has_value) {
            return _value != other._value;
        }
        return false;
    }

    bool operator>(optional const &other) const noexcept {
        if (!_has_value || !other._has_value) {
            return false;
        }
        return _value > other._value;
    }

    bool operator<(optional const &other) const noexcept {
        if (!_has_value || !other._has_value) {
            return false;
        }
        return _value < other._value;
    }

    bool operator>=(optional const &other) const noexcept {
        if (!_has_value || !other._has_value) {
            return true;
        }
        return _value >= other._value;
    }

    bool operator<=(optional const &other) const noexcept {
        if (!_has_value || !other._has_value) {
            return true;
        }
        return _value <= other._value;
    }

    void swap(optional &other) noexcept {
        if (_has_value && other._has_value) {
            using std::swap;
            swap(_value, other._has_value);
        } else if (!_has_value && !other._has_value) {
            // do nothing
        } else if (_has_value) {
            other.emplace(_value);
            reset();
        } else {
            emplace(std::move(other._value));
            other.reset();
        }
    }
};

#if __cpp_deduction_guides
template <class T> // C++17 才有 CTAD
optional(T) -> optional<T>;
#endif

template <typename T>
optional<std::decay_t<T>> make_optional(T &&value) {
    return optional<std::decay_t<T>>(std::forward<T>(value));
}

template <typename T, typename... Args>
optional<T> make_optional(Args &&...args) {
    return optional<T>(std::forward<Args>(args)...);
}
