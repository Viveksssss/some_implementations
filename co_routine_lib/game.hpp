#include "awaiters/awaiter.hpp"
#include "awaiters/Task.hpp"
#include "awaiters/when_any.hpp"
#include "utils/Scheduler.hpp"
#include <cstring>
#include <iostream>
#include <optional>
#include <random>
#include <unistd.h>
#include <utility>
char map[20][20];
int x = 10;
int y = 10;
int dx = 0;
int dy = 0;
bool running;
std::pair<int, int> p{5, 5};
int score = 0;

inline Task<void, co_async::EpollFilePromise> wait_file(
    co_async::EpollLoop &loop, int fileno, uint32_t events) {
    co_await co_async::EpollFileAwaiter(loop, fileno, events | EPOLLONESHOT);
}

void on_key(char c) {
    if (c == 'q') {
        running = false;
    } else if (c == 'a') {
        dx = -1;
        dy = 0;
    } else if (c == 'd') {
        dx = 1;
        dy = 0;
    } else if (c == 'w') {
        dx = 0;
        dy = -1;
    } else if (c == 's') {
        dx = 0;
        dy = 1;
    }
}

std::random_device rd;
std::mt19937 gen(rd());                     // Mersenne Twister 引擎
std::uniform_int_distribution<> dis(1, 19); // 1~100 均匀分布

std::pair<int, int> random_food() {
    int random_x = dis(gen);
    int random_y = dis(gen);
    return {random_x, random_y};
}

void on_draw() {
    std::memset(map, ' ', sizeof(map));
    map[y][x] = '@';
    map[p.first][p.second] = '*';
    std::string s = "\x1b[H\x1b[2J\x1b[3J";
    for (int i = 0; i < 20; ++i) {
        s += '#';
    }
    s += '\n';
    for (int i = 0; i < 20; ++i) {
        s += '#';
        for (int j = 0; j < 20; ++j) {
            s += map[i][j];
        }
        s += "#\n";
    }
    for (int i = 0; i < 20; ++i) {
        s += '#';
    }
    s += '\n';
    write(STDOUT_FILENO, s.data(), s.size());
}

void on_time() {
    if (x + dx >= 20 || x + dx < 0 || y + dy >= 20 || y + dy < 0) {
        running = false;
        exit(-1);
    }
    x += dx;
    y += dy;
    if (map[y][x] == '*') {
        score++;
        p = random_food();
    }
}

Task<std::optional<char>> get_char(int fd) {
    auto which = co_await when_any(
        wait_file(co_async::getEpollLoop(), STDIN_FILENO, EPOLLIN),
        std::this_thread::sleep_for(std::chrono::milliseconds(500)));
    if (which.index() == 0) {
        char c;
        ssize_t len = read(0, &c, 1);
        if (len == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) [[unlikely]] {
                throw std::system_error(errno, std::system_category());
            }
            exit(-1);
        }
        co_return c;
    } else {
        co_return std::nullopt;
    }
}

Task<void> async_main2() {
    while (true) {
        auto res = co_await get_char(STDIN_FILENO);
        if (res) {
            on_key(*res);
            on_time();
            on_draw();
        } else {
            on_time();
            on_draw();
        }
        std::cout << "x,y:" << x << ":" << y << std::endl;
        std::cout << "score:" << score << std::endl;
    }
}
