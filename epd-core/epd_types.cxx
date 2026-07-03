#include "epd_types.hxx"

namespace epd
{
    size::size(std::int32_t w, std::int32_t h)
        : width{w}, height{h}
    {

    }

    position::position(std::int32_t x, std::int32_t y)
        : x{x}, y{y}
    {

    }

    cursor::cursor(std::int32_t x, std::int32_t y, size displaySize)
        : x{x}, y{y}, m_displaySize{displaySize}
    {
    }

    cursor::cursor(position pos, size displaySize)
        : x{pos.x}, y{pos.y}, m_displaySize{displaySize}
    {
    }

    void cursor::next_line(std::int32_t yAdvance)
    {
        this->x = 0;
        this->y += yAdvance;
    }

    bool cursor::can_fit_glyph(std::int32_t glyphWidth) const
    {
        return (this->x + glyphWidth) <= this->m_displaySize.width;
    }

    void cursor::next_character(std::int32_t glyphWidth)
    {
        this->x += glyphWidth;
    }
}