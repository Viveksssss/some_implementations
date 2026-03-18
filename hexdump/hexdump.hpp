#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <type_traits>
#include <vector>

namespace hex {

template <typename Range>
    requires std::ranges::input_range<Range>
inline void hexdump(Range const &s, std::size_t one_line_num = 16) {
    using type = std::ranges::range_value_t<Range>;
    using UnsignedType = std::make_unsigned_t<type>; // 现在可以了！

    uint32_t addr = 0;
    std::vector<char> saved;
    for (auto chunk: s | std::views::chunk(one_line_num)) {
        std::cout << std::setw(8) << std::setfill('0') << std::hex << addr;
        for (auto const &c: chunk) {
            std::cout << " " << std::right << std::setw(2 * sizeof(type))
                      << std::hex << std::setfill('0')
                      << static_cast<unsigned long long>(
                             static_cast<UnsignedType>(c));
            ++addr;
            saved.push_back(c);
        }

        if constexpr (sizeof(type) == 1 && std::is_convertible_v<type, char>) {
            if (addr % one_line_num != 0) {
                for (std::size_t i = 0;
                     i < (one_line_num - addr % one_line_num) * 3; ++i) {
                    std::cout << " ";
                }
            }

            std::cout << " |";
            for (auto const &c0: saved) {
                char c = static_cast<unsigned char>(c0);
                if (std::isprint(c)) {
                    std::cout << c;
                } else {
                    std::cout << ".";
                }
            }
            std::cout << "|";
        }
        std::cout << "\n";
        saved.clear();
    }
}

} // namespace hex
