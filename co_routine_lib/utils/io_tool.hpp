#pragma once

#include "debug.hpp"
#include <source_location>
#include <string>
#include <system_error>
#include <termios.h>
#include <unistd.h>

namespace co_async {
int check_error(auto res,
    std::source_location const &loc = std::source_location::current()) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category(),
            (std::string)loc.file_name() + ":" + std::to_string(loc.line()));
    }
    return res;
}

#if !defined(NDEBUG)
auto checkError(auto res,
    std::source_location const &loc = std::source_location::current()) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category(),
            (std::string)loc.file_name() + ":" + std::to_string(loc.line()));
    }
    return res;
}

auto checkErrorNonBlock(auto res, int blockres = 0, int blockerr = EWOULDBLOCK,
    std::source_location const &loc = std::source_location::current()) {
    if (res == -1) {
        if (errno != blockerr) [[unlikely]] {
            throw std::system_error(errno, std::system_category(),
                (std::string)loc.file_name() + ":"
                    + std::to_string(loc.line()));
        }
        res = blockres;
    }
    return res;
}
#else
auto checkError(auto res) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category());
    }
    return res;
}

auto checkErrorNonBlock(
    auto res, int blockres = 0, int blockerr = EWOULDBLOCK) {
    if (res == -1) {
        if (errno != blockerr) [[unlikely]] {
            throw std::system_error(errno, std::system_category());
        }
        res = blockres;
    }
    return res;
}
#endif

}; // namespace co_async
inline struct termios g_orig_termios;

inline void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &g_orig_termios);
    struct termios raw = g_orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

inline void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
}
