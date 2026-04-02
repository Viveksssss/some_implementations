#pragma once

// #include "../awaiters/awaiter.hpp"
#include "../awaiters/Promise.hpp"
#include "../awaiters/Task.hpp"
#include "../utils/io_tool.hpp"
#include "debug.hpp"
#include "rbtree.hpp"
#include <chrono>
#include <coroutine>
#include <cstring>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <termios.h>
#include <thread>

namespace co_async {

struct TimerLoop {
    RbTree<SleepUntilPromise> timers;

    void add_timer(SleepUntilPromise &promise) {
        timers.insert(promise);
    }

    std::optional<std::chrono::system_clock::duration> run() {
        while (!timers.empty()) {
            auto now_time = std::chrono::system_clock::now();
            auto &promise = timers.front();
            if (promise.expireTime < now_time) {
                timers.erase(promise);
                std::coroutine_handle<SleepUntilPromise>::from_promise(promise)
                    .resume();
            } else {
                return promise.expireTime - now_time;
            }
        }
        return std::nullopt;
    }

    TimerLoop &operator=(TimerLoop &&) = delete;
};

inline TimerLoop &getTimerLoop() {
    static TimerLoop loop;
    return loop;
}

struct EpollLoop {
    bool add_listener(EpollFilePromise &promise) {
        struct epoll_event event;
        event.events = promise.events;
        event.data.ptr = &promise;
        // epoll_ctl(epfd, EPOLL_CTL_DEL, promise.fileno, nullptr);
        auto res = epoll_ctl(epfd, EPOLL_CTL_ADD, promise.fileno, &event);
        if (res == -1) {
            return false;
        } else {
            ++count;
        }
        return true;
    }

    void remove_listener(int fileno) {
        check_error(epoll_ctl(epfd, EPOLL_CTL_DEL, fileno, NULL));
        --count;
    }

    bool hasEvent() const noexcept {
        return count != 0;
    }

    bool run(std::optional<std::chrono::system_clock::duration> timeout) {
        struct epoll_event events[1024];
        auto timeoutInMs = -1;
        if (timeout) {
            timeoutInMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                *timeout)
                              .count();
        }
        int res = check_error(epoll_wait(epfd, events, 1024, timeoutInMs));

        for (int i = 0; i < res; ++i) {
            auto &event = events[i];
            auto &promise = *static_cast<EpollFilePromise *>(event.data.ptr);
            check_error(epoll_ctl(epfd, EPOLL_CTL_DEL, promise.fileno, NULL));
            std::coroutine_handle<EpollFilePromise>::from_promise(promise)
                .resume();
        }
        return true;
    }

    EpollLoop &operator=(EpollLoop &&) = delete;

    ~EpollLoop() {
        close(epfd);
    }
    struct epoll_event events[64];
    std::vector<std::coroutine_handle<>> handles;
    int epfd = check_error(epoll_create1(0));
    std::size_t count = 0;
};

inline EpollLoop &getEpollLoop() {
    static EpollLoop loop;
    return loop;
}

inline co_async::EpollFilePromise::~EpollFilePromise() {
    if (this->loop && fileno != -1) {
        epoll_ctl(loop->epfd, EPOLL_CTL_DEL, fileno, nullptr);
    }
}

struct AsyncLoop {
    AsyncLoop(EpollLoop &epollLoop, TimerLoop &timerLoop)
        : mEpollLoop(epollLoop)
        , mTimerLoop(timerLoop) { }

    bool run() {
        while (true) {
            auto timeout = mTimerLoop.run();
            if (mEpollLoop.hasEvent()) {
                mEpollLoop.run(timeout);
            } else if (timeout) {
                std::this_thread::sleep_for(*timeout);
            } else {
                return false;
            }
        }
        return true;
    }

    operator TimerLoop &() {
        return mTimerLoop;
    }

    operator EpollLoop &() {
        return mEpollLoop;
    }

private:
    TimerLoop &mTimerLoop;
    EpollLoop &mEpollLoop;
};

struct EpollFileAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(
        std::coroutine_handle<EpollFilePromise> coroutine) const noexcept {
        auto &promise = coroutine.promise();
        promise.fileno = fileno;
        promise.events = events;
        promise.loop = &loop;
        loop.add_listener(promise);
    }

    EpollEventMask await_resume() const noexcept {
        return resumeEvents;
    }

    using ClockType = std::chrono::system_clock;
    EpollLoop &loop;
    int fileno;
    uint32_t events;
    uint32_t resumeEvents;
};

struct [[nodiscard]] AsyncFile {
    AsyncFile() : fileno(-1) { }

    AsyncFile(int fileno) noexcept : fileno(fileno) { }

    AsyncFile(AsyncFile &&that) noexcept : fileno(that.fileno) {
        that.fileno = -1;
    }

    AsyncFile &operator=(AsyncFile &&that) noexcept {
        std::swap(fileno, that.fileno);
        return *this;
    }

    ~AsyncFile() {
        if (fileno != -1) {
            close(fileno);
        }
    }

    int fileNo() const noexcept {
        return fileno;
    }

    int release() noexcept {
        int ret = fileno;
        fileno = -1;
        return ret;
    }

    void setNonBlock() const {
        int flags = fcntl(fileno, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(fileno, F_SETFL, flags);
    }

private:
    int fileno;
};

inline Task<EpollEventMask, EpollFilePromise> wait_file_event(
    EpollLoop &loop, AsyncFile const &file, EpollEventMask events) {
    co_return co_await EpollFileAwaiter(loop, file.fileNo(), events);
}

inline std::size_t readFileSync(AsyncFile const &file, std::span<char> buffer) {
    return checkErrorNonBlock(
        read(file.fileNo(), buffer.data(), buffer.size()));
}

inline std::size_t writeFileSync(
    AsyncFile const &file, std::span<char const> buffer) {
    return checkErrorNonBlock(
        write(file.fileNo(), buffer.data(), buffer.size()));
}

inline bool need_async_wait(int fd) {
    struct stat st;
    fstat(fd, &st);
    // 普通文件和目录不需要 epoll
    return !S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode);
}

inline Task<std::size_t> read_file(
    EpollLoop &loop, AsyncFile const &file, std::span<char> buffer) {
    if (need_async_wait(file.fileNo())) {
        co_await wait_file_event(loop, file, EPOLLIN | EPOLLRDHUP);
    }
    auto len = readFileSync(file, buffer);
    co_return len;
}

inline Task<std::size_t> write_file(
    EpollLoop &loop, AsyncFile &file, std::span<char const> buffer) {
    if (need_async_wait(file.fileNo())) {
        co_await wait_file_event(loop, file, EPOLLOUT | EPOLLRDHUP);
    }
    auto len = writeFileSync(file, buffer);
    co_return len;
}

// inline Task<std::string> read_string(
//     EpollLoop &loop, co_async::AsyncFile &file) {
//     co_await wait_file_event(loop, file, EPOLLIN | EPOLLET);
//     std::string s;
//     std::size_t chunk_size = 8;
//     while (true) {
//         std::size_t exist = s.size();
//         s.resize(exist + chunk_size);
//         std::span<char> buffer(s.data() + exist, chunk_size);
//         auto len = co_await read_file(loop, file, buffer);
//         if (len != chunk_size) {
//             s.resize(exist + len);
//             break;
//         }
//         if (chunk_size < 65536) {
//             chunk_size *= 4;
//         }
//     }
//     co_return s;
// }

}; // namespace co_async
