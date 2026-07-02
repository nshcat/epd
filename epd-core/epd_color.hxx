#pragma once

#include <cstdint>

namespace epd
{
    enum class color
        : std::uint8_t
    {
        white = 0,
        black = 1
    };

    enum class color_mode
        : std::uint8_t
    {
        monochrome = 0,
        tricolor = 1,
    };
}