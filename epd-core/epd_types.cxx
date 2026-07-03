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

    position cursor::to_position() const
    {
        return position{this->x, this->y};
    }

    rectangle::rectangle(position location, size dimensions)
        : location{location}, dimensions{dimensions}
    {

    }

    rectangle rectangle::empty()
    {
        return rectangle{{0, 0}, {0, 0}};
    }

    void rectangle::pad_horizontal(std::int32_t padAmount)
    {
        // Adjust location of top left.
        this->location.x -= padAmount;

        // We need to add twice the padding amount to the width.
        this->dimensions.width += 2 * padAmount;
    }
        
    void rectangle::pad_vertical(std::int32_t padAmount)
    {
        // Adjust location of top left.
        this->location.y -= padAmount;

        // We need to add twice the padding amount to the height.
        this->dimensions.height += 2 * padAmount;
    }
}