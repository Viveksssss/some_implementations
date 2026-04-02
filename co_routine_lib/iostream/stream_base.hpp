#pragma once

#include "../awaiters/Task.hpp"
#include "../utils/Scheduler.hpp"
#include <concepts>
#include <iterator>
#include <span>

namespace co_async {

struct EOFException { };

template <typename Reader>
struct IStreamBase {
public:
    IStreamBase(std::size_t buffer_size = 8192)
        : buffer_size(buffer_size)
        , buffer(std::make_unique<char[]>(buffer_size)) { }

    IStreamBase(IStreamBase &&) = default;
    IStreamBase &operator=(IStreamBase &&) = default;

    Task<char> getchar() {
        if (buffer_empty()) {
            if (!co_await fill_buffer()) {
                throw EOFException();
            }
        }
        co_return buffer[index++];
    }

    Task<std::string> getline(char eol = '\n') {
        std::string s;
        while (true) {
            char c = co_await getchar();
            if (c == eol) {
                break;
            }
            s.push_back(c);
        }
        co_return s;
    }

    Task<std::string> getline(std::string_view eol) {
        std::string s;
        s.reserve(64);
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
        s.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            debug(), i;
            char c = co_await getchar();
            s.push_back(c);
        }
        co_return s;
    }

private:
    bool buffer_empty() const noexcept {
        return index == end;
    }

    Task<bool> fill_buffer() {
        auto *son = static_cast<Reader *>(this);
        end = co_await son->read(std::span(buffer.get(), buffer_size));
        index = 0;
        if (end == 0) [[unlikely]] {
            co_return false;
        }
        co_return true;
    }

    std::unique_ptr<char[]> buffer;
    std::size_t index = 0;
    std::size_t end = 0;
    std::size_t buffer_size;
};

template <typename Writer>
struct OStreamBase {
    explicit OStreamBase(std::size_t buffer_size = 8192)
        : buffer_size(buffer_size)
        , buffer(std::make_unique<char[]>(buffer_size)) { }

    OStreamBase(OStreamBase &&) = default;
    OStreamBase &operator=(OStreamBase &&) = default;

    Task<> flush() {
        if (index) [[likely]] {
            auto *son = static_cast<Writer *>(this);
            auto buf = std::span(buffer.get(), index);

            while (!buf.empty()) {
                auto len = co_await son->write(buf);
                if (len == 0) [[unlikely]] {
                    throw EOFException();
                }
                buf = buf.subspan(len);
            }
            index = 0;
        }
    }

    Task<> putchar(char c) {
        if (buffer_fulled()) {
            co_await flush();
        }
        buffer[index++] = c;
    }

    Task<> puts(std::string_view s) {
        // 批量写入优化
        if (index + s.size() <= buffer_size) {
            // 缓冲区足够，批量拷贝
            std::memcpy(buffer.get() + index, s.data(), s.size());
            index += s.size();
            co_return;
        }

        if (index != 0) {
            co_await flush();
        }

        auto *son = static_cast<Writer *>(this);
        while (!s.empty()) {
            auto len = co_await son->write(
                std::span<char const>(s.data(), s.size()));
            if (len == 0) {
                throw EOFException();
            }
            s = s.substr(len);
        }
    }

private:
    bool buffer_fulled() const noexcept {
        return index == buffer_size;
    }

    std::unique_ptr<char[]> buffer;
    std::size_t index = 0;
    std::size_t buffer_size = 0;
};

template <typename StreamBuf>
struct IOStreamBase : IStreamBase<StreamBuf>, OStreamBase<StreamBuf> {
    explicit IOStreamBase(std::size_t buffer_size = 8192)
        : IStreamBase<StreamBuf>(buffer_size)
        , OStreamBase<StreamBuf>(buffer_size) { }
};

template <typename StreamBuf>
struct [[nodiscard]] IStream : IStreamBase<StreamBuf>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit IStream(Args &&...args)
        : IStreamBase<IStream<StreamBuf>>()
        , StreamBuf(std::forward<Args>(args)...) { }

    IStream() = default;
};

template <class StreamBuf>
struct [[nodiscard]] OStream : OStreamBase<OStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit OStream(Args &&...args)
        : OStreamBase<OStream<StreamBuf>>()
        , StreamBuf(std::forward<Args>(args)...) { }

    OStream() = default;
};

template <class StreamBuf>
struct [[nodiscard]] IOStream : IOStreamBase<IOStream<StreamBuf>>, StreamBuf {
    template <class... Args>
        requires std::constructible_from<StreamBuf, Args...>
    explicit IOStream(Args &&...args)
        : IOStreamBase<IOStream<StreamBuf>>()
        , StreamBuf(std::forward<Args>(args)...) { }

    IOStream() = default;
};

}; // namespace co_async
