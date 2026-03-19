#include "hexdump.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

void print_usage() {
    std::cerr << "Options:\n"
              << "-f <file>   choose the file path\n"
              << "-x <size>   choose the one_line_size\n";
}

template <typename It>
struct Iter {
    It b, e;

    It begin() {
        return b;
    }

    It end() {
        return e;
    }

    It begin() const {
        return b;
    }

    It end() const {
        return e;
    }
};

int main(int argc, char **argv) {
    std::string context;
    std::ifstream file;
    std::size_t one_line_size = 16;

    for (std::size_t i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "-h") {
            print_usage();
            std::exit(0);
        } else if (arg == "-x") {
            if (++i >= argc) {
                throw std::runtime_error("-x requires an argument");
            }
            int size = std::stoi(argv[i]);
            if (!size % 8) {
                throw std::runtime_error("wrong num of one_line_size");
            }
            one_line_size = size;
        } else if (arg == "-f") {
            if (++i >= argc) {
                throw std::runtime_error("-f requires an argument");
            }
            auto path = std::filesystem::path(std::string(argv[i]));
            file.open(path);
            if (!file.good()) {
                std::cerr << std::strerror(errno) << " (" << errno << ") "
                          << path << std::endl;
                exit(0);
            }
        } else {
            throw std::runtime_error(std::string("Unknown option: ") +
                                     std::string(arg));
        }
    }

    if (!file.is_open()) {
        std::istreambuf_iterator<char> begin{std::cin}, end{};
        hex::hexdump(Iter{begin, end}, one_line_size);
    } else {
        std::istreambuf_iterator<char> begin{file}, end{};
        hex::hexdump(Iter{begin, end}, one_line_size);
    }
}
