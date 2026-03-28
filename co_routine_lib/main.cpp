#include "./awaiters/sleep.hpp"
#include "./awaiters/Task.hpp"
#include "./awaiters/when_all.hpp"
#include "./awaiters/when_any.hpp"
#include "./utils/debug.hpp"
#include "./utils/Scheduler.hpp"
#include "awaiters/awaiter.hpp"
#include "awaiters/Promise.hpp"
#include "utils/io_tool.hpp"
#include <chrono>
#include <fcntl.h>
#include <ranges>
#include <sys/epoll.h>
#include <unistd.h>

using namespace std::chrono_literals;

co_async::EpollLoop epollLoop;
co_async::TimerLoop timerLoop;

namespace co_async {

struct IstreamBase {
public:
    IstreamBase(co_async::AsyncFile &&file, std::size_t buffer_size = 8192)
        : file(std::move(file))
        , buffer(buffer_size) { }

    // , buffer(std::make_unique<char[]>(buffer_size)) { }

    Task<> fillBuffer() {
        end = co_await read_file(epollLoop, file, buffer);
        index = 0;
    }

    Task<char> getchar() {
        if (index == end) {
            co_await fillBuffer();
        }
        co_return buffer[index++];
    }

    Task<std::string> getline(char eol = '\n') {
        std::string s;
        while (true) {
            char c = co_await getchar();
            s.push_back(c);
            if (c == eol) {
                break;
            }
        }
        co_return s;
    }

    Task<std::string> getline(std::string_view eol) {
        std::string s;
        while (true) {
            char c = co_await getchar();
            s.push_back(c);
            if (s.ends_with(eol)) {
                break;
            }
        }
        co_return s;
    }

    Task<std::string> getn(std::size_t n) {
        std::string s;
        while (true) {
            char c = co_await getchar();
            s.push_back(c);
            if (s.size() >= n) {
                break;
            }
        }
        co_return s;
    }

private:
    co_async::AsyncFile file;
    // std::unique_ptr<char[]> buffer;
    std::vector<char> buffer;
    std::size_t index = 0;
    std::size_t end = 0;
};

} // namespace co_async

Task<void> async_main() {
    co_async::AsyncFile file(STDIN_FILENO);
    co_async::IstreamBase is(std::move(file));
    while (true) {
        auto s = co_await is.getline(':');
        debug(), s;
        s = co_await is.getline("\r\n");
        debug(), s;
    }
}

int main() {
    // enable_raw_mode();

    co_async::AsyncLoop loop(epollLoop, timerLoop);
    auto t = async_main();
    co_async::run_task(loop, t);

    disable_raw_mode();

    return 0;
}
