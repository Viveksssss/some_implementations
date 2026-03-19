#include "optional.hpp"
#include <iostream>
#include <vector>

auto functory(int a) -> optional<int> {
    if (a > 10) {
        return optional<int>(a);
    } else {
        return nullopt;
    }
}

struct ss {
    int a;
    int b;

    ss(int a, int b) : a(a), b(b) {}
};

int main(int, char **) {
    optional<std::vector<int>> o1({1, 2, 3, 4, 54, 6, 7, 8});
    if (o1) {
        for (auto &p: o1.value()) {
            std::cout << p << " ";
        }
    }
    std::cout << std::endl;
    auto o2 = make_optional<std::vector<int>>({2, 5, 7, 34, 3, 7, 89, 4, 2});
    if (o2) {
        for (auto &p: o2.value()) {
            std::cout << p << " ";
        }
    }

    std::cout << std::endl;

    optional<std::vector<int>> o3({1, 2, 3, 4, 5, 6, 7, 8, 9});
    if (o3) {
        for (auto &p: o3.value()) {
            std::cout << p << " ";
        }
    }

    optional<ss> o4(inplace, 2, 3);

    long long int a;
    while (std::cin >> a) {
        auto p = functory(a);
        if (p || p.has_value()) {
            std::cout << "*p:" << *p << std::endl;
            std::cout << "p.value():" << p.value() << std::endl;
        } else {
            std::cout << "p.has_value:" << p.has_value() << std::endl;
            std::cout << "p.valur_or(65):" << p.value_or(65) << std::endl;
        }
    }
}
