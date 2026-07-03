#pragma once

#include <cstdint>

namespace epd
{
    // 2D size/dimensions
    struct size
    {
    public:
        size(std::int32_t width, std::int32_t height);

    public:
        std::int32_t width;
        std::int32_t height;
    };

    // 2D position
    struct position
    {
    public:
        position(std::int32_t x, std::int32_t y);

    public:
        std::int32_t x;
        std::int32_t y;
    };

    // Struct representing the current font glyph drawing
    // location. Supports line wrapping and other helpful operations.
    struct cursor
    {
    public:
        cursor(std::int32_t x, std::int32_t y, size displaySize);
        cursor(position pos, size displaySize);

    public:
        // Advance to next line.
        void next_line(std::int32_t yAdvance);
        bool can_fit_glyph(std::int32_t glyphWidth) const;
        // Advance cursor behind the last drawn character.
        void next_character(std::int32_t glyphWidth);
    
    public:
        std::int32_t x;
        std::int32_t y;

    protected:
        size m_displaySize;
    };
}