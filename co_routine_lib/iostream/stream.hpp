#pragma once

#include "../utils/Scheduler.hpp"
#include "stdio.hpp"
#include "stream_base.hpp"
#include <algorithm>
#include <span>
#include <string_view>

namespace co_async {

struct FileBuf {
    EpollLoop *loop;
    AsyncFile file;

    FileBuf(EpollLoop &loop, AsyncFile &&file)
        : loop(&loop)
        , file(std::move(file)) { }

    FileBuf(EpollLoop &loop, int fd) : loop(&loop), file(AsyncFile(fd)) { }

    FileBuf() noexcept : loop(nullptr) { }

    Task<std::size_t> read(std::span<char> buffer) {
        return read_file(*loop, file, buffer);
    }

    Task<std::size_t> write(std::span<char const> buffer) {
        return write_file(*loop, file, buffer);
    }
};

struct StdioBuf {
    EpollLoop *loop;
    AsyncFile fileIn;
    AsyncFile fileOut;

    StdioBuf(EpollLoop &loop)
        : loop(&loop)
        , fileIn(async_stdin(true))
        , fileOut(async_stdout()) { }

    StdioBuf(EpollLoop &loop, AsyncFile &&fileIn, AsyncFile &&fileOut)
        : loop(&loop)
        , fileIn(std::move(fileIn))
        , fileOut(std::move(fileOut)) { }

    StdioBuf() noexcept : loop(nullptr) { }

    Task<std::size_t> read(std::span<char> buffer) {
        return read_file(*loop, fileIn, buffer);
    }

    Task<std::size_t> write(std::span<char const> buffer) {
        return write_file(*loop, fileOut, buffer);
    }
};

struct StringReadBuf {
    std::string_view stringview;
    std::size_t pos;

    StringReadBuf() noexcept : pos(0) { }

    StringReadBuf(std::string_view strView) : stringview(strView), pos(0) { }

    Task<std::size_t> read(std::span<char> buffer) {
        std::size_t size = std::min(buffer.size(), stringview.size() - pos);
        std::copy_n(stringview.begin() + pos, size, buffer.begin());
        pos += size;
        co_return size;
    }
};

struct StringWriteBuf {
    std::string str;

    StringWriteBuf() noexcept { }

    StringWriteBuf(std::string &&str) : str(std::move(str)) { }

    Task<std::size_t> write(std::span<char const> buffer) {
        str.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }
};

using FileIStream = IStream<FileBuf>;
using FileOStream = OStream<FileBuf>;
using FileStream = IOStream<FileBuf>;
using StdioStream = IOStream<StdioBuf>;
using StringIStream = IStream<StringReadBuf>;
using StringOStream = OStream<StringWriteBuf>;

} // namespace co_async
