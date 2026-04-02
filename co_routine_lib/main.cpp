#include "./awaiters/sleep.hpp"
#include "./awaiters/Task.hpp"
#include "./awaiters/when_all.hpp"
#include "./awaiters/when_any.hpp"
#include "./utils/debug.hpp"
#include "./utils/Scheduler.hpp"
#include "awaiters/awaiter.hpp"
#include "awaiters/Promise.hpp"
#include "iostream/stdio.hpp"
#include "iostream/stream.hpp"
#include "net/socket.hpp"
#include "utils/io_tool.hpp"
#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <liburing.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ranges>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std::chrono_literals;

co_async::EpollLoop epollLoop;
co_async::TimerLoop timerLoop;

Task<void> async_main() {
    int fd = open("/home/vivek/ww.html", O_RDONLY);
    co_async::AsyncFile file(fd);
    co_async::FileStream s(epollLoop, std::move(file));
    try {
        while (true) {
            std::string line = co_await s.getline("\\r\\n");
            debug(), line;
        }
    } catch (...) {
        debug(), "End of file reached";
    }
}

Task<> handle_connection(co_async::AsyncFile conn) {
    debug(), "Gets:";
    co_async::FileStream s(epollLoop, std::move(conn));
    while (true) {
        auto l = co_await s.getline("\r\n");
        debug(), l;
        if (l == "\r\n") {
            break;
        }
    }
    debug(), "发来泥码被";
    co_await s.puts("HTTP/1.1 200 OK\r\n");
    co_await s.puts("Content-Type: text/plain\r\n");
    co_await s.puts("Content-Length: 12\r\n");
    co_await s.puts("\r\n");
    co_await s.puts("Hello, world");
    co_await s.flush();
    co_return;
}

Task<> admin() {
    auto server = co_await co_async::create_tcp_server(epollLoop,
        co_async::socket_address(co_async::ip_address("127.0.0.1"), 9999));
    co_async::socket_listen(server);

    while (true) {
        auto [conn, addr]
            = co_await co_async::socket_accept<co_async::IpAddress>(
                epollLoop, server);

        debug(), addr;
        co_await handle_connection(std::move(conn));
    }
}

int main() {
    co_async::AsyncLoop loop(epollLoop, timerLoop);
    auto t = admin();
    co_async::run_task(loop, t);

    // io_uring ring;
    // io_uring_queue_init(32, &ring, 0);

    // io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    // io_uring_sqe_set_data64(sqe, 999);
    // char buf[6];
    // io_uring_prep_read(sqe, STDIN_FILENO, buf, sizeof(buf), 0);
    // io_uring_submit(&ring);

    // io_uring_cqe *cqe;
    // io_uring_wait_cqe(&ring, &cqe);

    // debug(), cqe->res;
    // debug(), (int64_t)io_uring_cqe_get_data64(cqe);

    // io_uring_cqe_seen(&ring, cqe);

    // io_uring_queue_exit(&ring);

    return 0;
}
