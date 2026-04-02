#pragma once

#include "../awaiters/Task.hpp"
#include "../utils/io_tool.hpp"
#include "../utils/Scheduler.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <system_error>
#include <unistd.h>
#include <variant>

namespace co_async {

struct IpAddress {
    IpAddress(in_addr addr) noexcept : addr(addr) { }

    IpAddress(in6_addr addr) noexcept : addr(addr) { }

    IpAddress() = default;
    std::variant<in_addr, in6_addr> addr;
};

inline IpAddress ip_address(char const *ip) {
    in_addr addr = {};
    in6_addr addr6 = {};
    if (checkError(inet_pton(AF_INET, ip, &addr))) {
        return addr;
    } else if (checkError(inet_pton(AF_INET6, ip, &addr6))) {
        return addr6;
    }

    /* 域名解析 */
    hostent *hent = gethostbyname(ip);
    for (int i = 0; hent->h_addr_list[i]; i++) {
        if (hent->h_addrtype == AF_INET) {
            std::memcpy(&addr, hent->h_addr_list[i], sizeof(in_addr));
            return addr;
        } else {
            std::memcpy(&addr6, hent->h_addr_list[i], sizeof(addr6));
            return addr6;
        }
    }

    throw std::invalid_argument("invalid domain name or ip address");
}

struct SocketAddress {
    SocketAddress() = default;

    SocketAddress(char const *path) {
        sockaddr_un saddr = {};
        saddr.sun_family = AF_UNIX;
        std::strncpy(saddr.sun_path, path, sizeof(saddr.sun_path) - 1);
        std::memcpy(&addr, &saddr, sizeof(saddr));
        addr_len = sizeof(saddr);
    }

    SocketAddress(in_addr host, int port) {
        sockaddr_in saddr = {};
        saddr.sin_family = AF_INET;
        std::memcpy(&saddr.sin_addr, &host, sizeof(saddr.sin_addr));
        saddr.sin_port = htons(port);
        std::memcpy(&addr, &saddr, sizeof(saddr));
        addr_len = sizeof(saddr);
    }

    SocketAddress(in6_addr host, int port) {
        sockaddr_in6 saddr = {};
        saddr.sin6_family = AF_INET6;
        std::memcpy(&saddr.sin6_addr, &host, sizeof(saddr.sin6_addr));
        saddr.sin6_port = htons(port);
        std::memcpy(&addr, &saddr, sizeof(saddr));
        addr_len = sizeof(saddr);
    }

    sockaddr_storage addr;
    socklen_t addr_len;
};

inline AsyncFile create_udp_socket(SocketAddress const &addr) {
    return AsyncFile(socket(addr.addr.ss_family, SOCK_DGRAM, 0));
}

inline SocketAddress socket_address(IpAddress ip, int port) {
    return std::visit(
        [&](auto const &addr) { return SocketAddress(addr, port); }, ip.addr);
}

inline SocketAddress get_address_from_socket(AsyncFile &sock) {
    SocketAddress sa;
    sa.addr_len = sizeof(sa.addr);
    checkError(getsockname(sock.fileNo(), (sockaddr *)&sa.addr, &sa.addr_len));
    return sa;
}

template <typename T>
inline T get_option_from_socket(AsyncFile &sock, int level, int opt) {
    T opt_val;
    socklen_t opt_len = sizeof(opt_val);
    checkError(
        getsockopt(sock.fileNo(), level, opt, (sockaddr *)&opt_val, &opt_len));
    return opt_val;
}

template <typename T>
inline void set_socket_option(
    AsyncFile &sock, int level, int opt, T const &opt_val) {
    checkError(
        setsockopt(sock.fileNo(), level, opt, &opt_val, sizeof(opt_val)));
}

inline Task<void> socket_connect(
    EpollLoop &loop, AsyncFile &sock, SocketAddress const &addr) {
    sock.setNonBlock();
    int res = checkErrorNonBlock(
        connect(sock.fileNo(), (sockaddr const *)&addr.addr, addr.addr_len), -1,
        EINPROGRESS);
    if (res == -1) [[likely]] {
        co_await wait_file_event(loop, sock, EPOLLOUT | EPOLLRDHUP);
        int err = get_option_from_socket<int>(sock, SOL_SOCKET, SO_ERROR);
        if (err != 0) [[unlikely]] {
            throw std::system_error(err, std::system_category(), "connect");
        }
    }
}

inline Task<AsyncFile> create_tcp_client(
    EpollLoop &loop, SocketAddress const &addr) {
    AsyncFile sock(socket(addr.addr.ss_family, SOCK_STREAM, 0));
    co_await socket_connect(loop, sock, addr);
    co_return sock;
}

inline Task<void> socket_bind(EpollLoop &loop, AsyncFile &sock,
    SocketAddress const &addr, int backlog = SOMAXCONN) {
    sock.setNonBlock();
    int reuse = 1;
    set_socket_option(sock, SOL_SOCKET, SO_REUSEADDR, &reuse);
    checkError(
        bind(sock.fileNo(), (sockaddr const *)&addr.addr, addr.addr_len));
    co_await wait_file_event(loop, sock, EPOLLOUT | EPOLLRDHUP);
    int err = get_option_from_socket<int>(sock, SOL_SOCKET, SO_ERROR);
    if (err != 0) [[unlikely]] {
        throw std::system_error(err, std::system_category(), "bind");
    }
}

inline Task<AsyncFile> create_tcp_server(
    EpollLoop &loop, SocketAddress const &addr) {
    AsyncFile sock(socket(addr.addr.ss_family, SOCK_STREAM, 0));
    sock.setNonBlock();
    co_await socket_bind(loop, sock, addr);
    co_return sock;
}

inline void socket_listen(AsyncFile &sock, int backlog = SOMAXCONN) {
    checkError(listen(sock.fileNo(), backlog));
}

inline void socket_shutdown(AsyncFile &sock, int flags = SHUT_RDWR) {
    checkError(shutdown(sock.fileNo(), flags));
}

template <typename AddrType>
inline Task<std::tuple<AsyncFile, AddrType>> socket_accept(
    EpollLoop &loop, AsyncFile &sock) {
    struct sockaddr_storage sock_addr;
    socklen_t addr_len = sizeof(sock_addr);
    co_await wait_file_event(loop, sock, EPOLLIN | EPOLLRDHUP);
    int res = checkError(accept4(
        sock.fileNo(), (sockaddr *)&sock_addr, &addr_len, SOCK_NONBLOCK));

    AddrType addr;
    if (sock_addr.ss_family == AF_INET) {
        addr = ((sockaddr_in *)&sock_addr)->sin_addr;
    } else if (sock_addr.ss_family == AF_INET6) {
        addr = ((sockaddr_in6 *)&sock_addr)->sin6_addr;
    } else [[unlikely]] {
        throw std::runtime_error("unknown address family");
    }
    co_return std::make_tuple(std::move(AsyncFile(res)), addr);
}

} // namespace co_async
