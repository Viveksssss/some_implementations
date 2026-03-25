#pragma once

#include <tuple>

template <typename... Ts>
auto co_result(std::tuple<Ts...> t) {
    int sum = 0;
    std::apply(
        [&sum](auto &&...args) {
            (([&sum](auto &&arg) {
                if constexpr (std::convertible_to<decltype(arg), int>) {
                    sum += arg;
                }
            }(std::forward<decltype(args)>(args))),
                ...);
        },
        t);
    return sum;
}
