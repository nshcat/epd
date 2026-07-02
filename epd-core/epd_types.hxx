#pragma once

#include <cstdint>

namespace epd
{
    struct position
    {
    public:
        position(std::int32_t x, std::int32_t y);

    public:
        std::int32_t x;
        std::int32_t y;
    };
}