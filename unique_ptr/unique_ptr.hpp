#pragma once

#include <type_traits>
#include <utility>

/* 删除器 */
template <typename _Tp>
struct DefaultDeleter {
    void operator()(_Tp *p) const {
        delete p;
    }
};

template <typename _Tp>
struct DefaultDeleter<_Tp[]> {
    void operator()(_Tp *p) const {
        delete[] p;
    }
};

template <class _Tp, class _Deleter = DefaultDeleter<_Tp>>
class unique_ptr {
private:
    _Tp *_m_p;
    [[no_unique_address]] _Deleter _m_deleter;

    template <class _Up, class _UDeleter>
    friend class unique_ptr;

public:
    using element_type = _Tp;
    using pointer = _Tp *;
    using delete_type = _Deleter;

    unique_ptr(std::nullptr_t = nullptr) noexcept : _m_p(nullptr) {}

    explicit unique_ptr(_Tp *p) noexcept : _m_p(p) {}

    template <class _Up, class _UDeleter,
              class = std::enable_if<std::is_convertible_v<_Up *, _Tp *>>>
    unique_ptr(unique_ptr<_Up, _UDeleter> &&_that) noexcept
        : _m_p(_that.release()) {}

    // template <class _Up, class _UDeleter,
    //           class = std::enable_if<std::is_convertible_v<_Tp *, _Tp *> &&
    //                                  !std::is_same_v<_Up, _Tp>>>
    // operator unique_ptr<_Up, _UDeleter>() && {
    //     return unique_ptr<_Up, _UDeleter>(release(),
    //                                       std::forward<_UDeleter>(_m_deleter));
    // }

    ~unique_ptr() noexcept {
        if (_m_p) {
            _m_deleter(_m_p);
        }
    }

    unique_ptr(unique_ptr const &_that) = delete;            // 拷贝构造函数
    unique_ptr &operator=(unique_ptr const &_that) = delete; // 拷贝赋值函数

    unique_ptr(unique_ptr &&_that) noexcept : _m_p(_that._m_p) {
        _that._m_p = nullptr;
    }

    unique_ptr &operator=(unique_ptr &&_that) noexcept {
        if (this != &_that) [[likely]] {
            if (_m_p) {
                _m_deleter(_m_p);
            }
            _m_p = _that._m_p;
            _that._m_p = nullptr;
        }
        return *this;
    }

    void swap(unique_ptr &_that) noexcept {
        std::swap(_m_p, _that._m_p);
        std::swap(_m_deleter, _that._m_deleter);
    }

    _Tp *get() const noexcept {
        return _m_p;
    }

    _Tp *operator->() noexcept {
        return _m_p;
    }

    _Tp &operator*() {
        return *_m_p;
    }

    _Deleter get_delete() const noexcept {
        return _m_deleter;
    }

    _Tp *release() noexcept {
        _Tp *p = _m_p;
        _m_p = nullptr;
        return p;
    }

    void reset(_Tp *p = nullptr) noexcept {
        if (_m_p) {
            _m_deleter(_m_p);
        }
        _m_p = p;
    }

    explicit operator bool() const noexcept {
        return _m_p != nullptr;
    }

    bool operator==(unique_ptr const &_that) const noexcept {
        return _m_p == _that._m_p;
    }

    bool operator!=(unique_ptr const &_that) const noexcept {
        return _m_p != _that._m_p;
    }
};

template <class _Tp, class _Deleter>
class unique_ptr<_Tp[], _Deleter> : protected unique_ptr<_Tp, _Deleter> {
    using unique_ptr<_Tp, _Deleter>::unique_ptr;
};

template <class _Tp, class... _Args,
          std::enable_if_t<!std::is_unbounded_array_v<_Tp>, int> = 0>
unique_ptr<_Tp> make_unique(_Args &&..._args) {
    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(_args)...));
}

template <class _Tp, std::enable_if_t<!std::is_unbounded_array_v<_Tp>, int> = 0>
unique_ptr<_Tp> make_unique_for_overwrite() {
    return unique_ptr<_Tp>(new _Tp);
}

template <class _Tp, std::enable_if_t<std::is_unbounded_array_v<_Tp>, int> = 0>
unique_ptr<_Tp> make_unique(std::size_t _len) {
    return unique_ptr<_Tp>(new std::remove_extent_t<_Tp>[_len]());
}

template <class _Tp, std::enable_if_t<std::is_unbounded_array_v<_Tp>, int> = 0>
unique_ptr<_Tp> make_unique_for_overwrite(std::size_t _len) {
    return UniquePtr<_Tp>(new std::remove_extent_t<_Tp>[_len]);
}
