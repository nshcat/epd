#pragma once

#include <cstdint>

namespace epd
{
    struct position
    {
    public:
        position(std::uint32_t x, std::uint32_t y);

    public:
        std::uint32_t x;
        std::uint32_t y;
    };
}