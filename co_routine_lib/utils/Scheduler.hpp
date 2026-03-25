#pragma once

#include "../awaiters/Promise.hpp"
#include "../utils/io_tool.hpp"
#include "rbtree.hpp"
#include <chrono>
#include <coroutine>
#include <sys/epoll.h>
#include <thread>

struct Scheduler {
    RbTree<SleepUntilPromise> timers;

    void add_timer(SleepUntilPromise &promise) {
        timers.insert(promise);
    }

    void run(std::coroutine_handle<> coroutine) {
        while (!coroutine.done()) {
            coroutine.resume();
            while (!timers.empty()) {
                auto now_time = std::chrono::system_clock::now();
                auto &promise = timers.front();
                if (promise.expireTime < now_time) {
                    timers.erase(promise);
                    std::coroutine_handle<SleepUntilPromise>::from_promise(
                        promise)
                        .resume();
                } else {
                    std::this_thread::sleep_until(promise.expireTime);
                }
            }
        }
    }

    Scheduler &operator=(Scheduler &&) = delete;
};

inline Scheduler &getLoop() {
    static Scheduler loop;
    return loop;
}

namespace co_async {
    
struct EpollLoop {
    void add_listener(EpollFilePromise &promise) {
        struct epoll_event event;
        event.events = promise.events;
        event.data.ptr = &promise;
        check_error(epoll_ctl(epfd, EPOLL_CTL_ADD, promise.fileno, &event));
    }

    void run() {
        struct epoll_event events[1024];
        int res = check_error(epoll_wait(epfd, events, 1024, -1));
        for (int i = 0; i < res; ++i) {
            auto &event = events[i];
            auto &promise = *static_cast<EpollFilePromise *>(event.data.ptr);
            check_error(epoll_ctl(epfd, EPOLL_CTL_DEL, promise.fileno, NULL));
            std::coroutine_handle<EpollFilePromise>::from_promise(promise)
                .resume();
        }
    }

    EpollLoop &operator=(EpollLoop &&) = delete;

    ~EpollLoop() {
        close(epfd);
    }

    int epfd = check_error(epoll_create1(0));
};
}; // namespace co_async
