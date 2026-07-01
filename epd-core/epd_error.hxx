#pragma once

#include <cstdint>

#define EPD_OK 0
#define EPD_IS_ERR(x) ((x) != EPD_OK)
#define EPD_CHECK_ERR(x) do{ if(EPD_IS_ERR(x)) { return (x); } } while(0)

namespace epd
{
    using error_t = std::uint32_t;
}