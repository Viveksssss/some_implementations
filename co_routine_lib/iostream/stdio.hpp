#pragma once

#include "../utils/Scheduler.hpp"
#include <unistd.h>

namespace co_async {

/*
    如果不使用dup(fileno):

    问题1：stdin 被设置了非阻塞模式，影响所有使用 stdin 的代码
    问题2：当 file 析构时，会 close(0)，导致标准输入被关闭！
*/
inline AsyncFile async_stdfile(int fileno) {
    AsyncFile file(checkError(dup(fileno)));
    file.setNonBlock();
    return file;
}

inline AsyncFile async_stdin(bool noCanon = false, bool noEcho = false) {
    AsyncFile file = async_stdfile(STDIN_FILENO);
    if ((noCanon || noEcho) && isatty(file.fileNo())) {
        struct termios tc;
        tcgetattr(file.fileNo(), &tc);
        if (noCanon) {
            tc.c_lflag &= ~ICANON;
        }
        if (noEcho) {
            tc.c_lflag &= ~ECHO;
        }
        tcsetattr(file.fileNo(), TCSANOW, &tc);
    }
    return file;
}

inline AsyncFile async_stdout() {
    return async_stdfile(STDOUT_FILENO);
}

inline AsyncFile async_stderr() {
    return async_stdfile(STDERR_FILENO);
}

} // namespace co_async
