#include "./awaiters/sleep.hpp"
#include "./awaiters/Task.hpp"
#include "./awaiters/when_all.hpp"
#include "./awaiters/when_any.hpp"
#include "./utils/debug.hpp"
#include "./utils/Scheduler.hpp"
#include "awaiters/awaiter.hpp"
#include "awaiters/Promise.hpp"
#include <any>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <sys/epoll.h>
#include <system_error>
#include <thread>
#include <unistd.h>

using namespace std::chrono_literals;

inline Task<void, co_async::EpollFilePromise> wait_file(
    co_async::EpollLoop &loop, int fileno, uint32_t events) {
    co_await co_async::EpollFileAwaiter(loop, fileno, events);
}

co_async::EpollLoop loop;

Task<std::string> reader(int fd) {
    co_await wait_file(loop, fd, EPOLLIN);
    std::string s;
    while (true) {
        char c;
        ssize_t len = read(0, &c, 1);
        if (len == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) [[unlikely]] {
                throw std::system_error(errno, std::system_category());
            }
            break;
        }
        s.push_back(c);
    }
    co_return s;
}

Task<void> async_main() {
    while (true) {
        auto s = co_await reader(STDIN_FILENO);
        debug(), "Gets:", s;
        if (s == "quit\n") {
            break;
        }
    }
}

int main() {

    int flags = fcntl(STDIN_FILENO, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(STDIN_FILENO,F_SETFL,flags);

    auto t = async_main();
    t.mCoroutine.resume();
    while(!t.mCoroutine.done()){
        loop.run();
    }


    return 0;
}
