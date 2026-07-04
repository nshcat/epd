#include <cstdlib>
#include <utility>

#include "epd_graphics.hxx"
#include "epd_error.hxx"
#include "epd_font.hxx"
#include "epd_panel.hxx"
#include "epd_types.hxx"

namespace epd
{
    namespace internal
    {
        text_measurement_state::text_measurement_state(cursor location)
            : location{location}, initialLocation(location.to_position())
        {

        }

        rectangle text_measurement_state::to_rectangle() const
        {
            position topLeft = this->initialLocation;
            size dimensions{0, 0};

            // Check if we were able to calculate sensible
            // values for x and y range.
            // If so, apply them to the rectangle dimensions.
            if(this->maxx >= this->minx)
            {
                topLeft.x = this->minx;
                dimensions.width = (this->maxx - this->minx + 1);
            }

            if(this->maxy >= this->miny)
            {
                topLeft.y = this->miny;
                dimensions.height = (this->maxy - this->miny + 1);
            }

            return rectangle{ topLeft, dimensions };
        }
    }

    graphics::graphics(panel* panel, epd::rotation rotation)
        : m_panel{panel}, m_transport{panel->transport()}
    {
        this->set_rotation(rotation);
    }

    void graphics::set_rotation(epd::rotation rotation)
    {
        // Adjust virtual width and height based on new rotation
        switch(rotation)
        {
            case epd::rotation::by_0deg:
            case epd::rotation::by_180deg:
                this->m_width = this->m_panel->width();
                this->m_height = this->m_panel->height();
                break;

            case epd::rotation::by_90deg:
            case epd::rotation::by_270deg:
                this->m_height = this->m_panel->width();
                this->m_width = this->m_panel->height();
                break;

            default:
                return;
        }

        this->m_rotation = rotation;
    }

    epd::rotation graphics::current_rotation() const
    {
        return this->m_rotation;
    }

    std::int32_t graphics::width() const
    {
        return this->m_width;
    }

    std::int32_t graphics::height() const
    {
        return this->m_height;
    }

    size graphics::dimensions() const
    {
        return size{this->m_width, this->m_height};
    }

    void graphics::clear(color color)
    {
        this->m_panel->fill(color);
    }

    error_t graphics::display()
    {
        error_t result{EPD_OK};

        result = this->m_panel->refresh();
        EPD_CHECK_ERR(result);

        result = this->m_panel->sleep();
        EPD_CHECK_ERR(result);

        return EPD_OK;
    }

    void graphics::clear_partial(const rectangle& area, color color)
    {
        this->fill_rect(area, color);
    }

    error_t graphics::display_partial(const rectangle& area)
    {
        // Empty refresh area is not okay
        if(area.dimensions.height <= 0 || area.dimensions.width <= 0)
        {
            return EPD_FAIL;
        }

        // Panel might not support partial refresh
        if(!this->m_panel->supports_partial_refresh())
        {
            return EPD_FAIL;
        }

        error_t result{EPD_OK};

        // Adjust refresh area for current rotation
        const auto adjustedArea = this->map_rectangle_to_panel(area);

        result = this->m_panel->partial_refresh(adjustedArea);
        EPD_CHECK_ERR(result);

        result = this->m_panel->sleep();
        EPD_CHECK_ERR(result);

        return EPD_OK;
    }

    void graphics::draw_pixel(const position& position, color color)
    {
        // Perform bounds check to avoid out-of-bounds buffer access
        if(position.x >= this->width() || position.y >= this->height())
        {
            return;
        }

        // Rotate coordinates based on current rotation mode
        const auto adjustedPostion = this->map_point_to_panel(position);

        // Write to panel framebuffer
        this->m_panel->set_pixel(adjustedPostion, color);
    }

    void graphics::draw_hline(const position& position, std::int32_t width, color color)
    {
        epd::position to{ position.x + static_cast<std::int32_t>(width) - 1, position.y };
        this->draw_line(position, to, color);
    }

    void graphics::draw_vline(const position& position, std::int32_t height, color color)
    {
        epd::position to{ position.x, position.y + static_cast<std::int32_t>(height) - 1 };
        this->draw_line(position, to, color);
    }

    void graphics::draw_line(const position& from_, const position& to_, color color)
    {
        // Make sure to yield so that overeager watch dog timers dont fuck us up
        // (Especially common with ESP32..)
        this->m_transport->yield();

        position from{from_};
        position to{to_};

        // Bresenhams algorithm, stolen straight from Wikipedia :^)
        const auto isSteep = abs(to.y - from.y) > abs(to.x - from.x);

        if(isSteep)
        {
            std::swap(from.x, from.y);
            std::swap(to.x, to.y);
        }

        if(from.x > to.x)
        {
            std::swap(from.x, to.x);
            std::swap(from.y, to.y);
        }

        std::int32_t dx{ to.x - from.x };
        std::int32_t dy{ abs(to.y - from.y) };

        std::int32_t error = dx / 2;
        std::int32_t yStep{ (from.y < to.y) ? 1 : -1 };

        for(; from.x <= to.x; from.x++)
        {
            if(isSteep)
            {
                this->draw_pixel(position{from.y, from.x}, color);
            }
            else
            {
                this->draw_pixel(from, color);
            }

            error -= dy;

            if(error < 0)
            {
                from.y += yStep;
                error += dx;
            }
        }
    }

    void graphics::draw_rect(const rectangle& rect, color color)
    {
        const auto& topLeft = rect.location;
        const auto& dimensions = rect.dimensions;

        this->draw_hline(topLeft, dimensions.width, color);
        this->draw_hline(epd::position{topLeft.x, topLeft.y + (std::int32_t)dimensions.height - 1}, dimensions.width, color);

        this->draw_vline(topLeft, dimensions.height, color);
        this->draw_vline(epd::position{topLeft.x + (std::int32_t)dimensions.width - 1, topLeft.y}, dimensions.height, color);
    }

    void graphics::fill_rect(const rectangle& rect, color color)
    {
        // We do not draw empty rectangles!
        if(rect.dimensions.width <= 0 || rect.dimensions.height <= 0)
        {
            return;
        }

        for(std::int32_t ix = 0; ix < rect.dimensions.width; ++ix)
        {
            for(std::int32_t iy = 0; iy < rect.dimensions.height; ++iy)
            {
                this->draw_pixel({rect.location.x + ix, rect.location.y + iy}, color);
            }
        }
    }

    cursor graphics::draw_text(const font* font, const position& position, std::string_view string, color color)
    {
        return this->draw_text(font, cursor{position, this->dimensions()}, string, color);
    }

    cursor graphics::draw_text(const font* font, const cursor& startPosition, std::string_view string, color color)
    {
        if(!font) // Bad pointer? For real? :'(
        {
            return startPosition;
        }

        // Dont draw if we are already out of bounds.
        if(startPosition.x >= this->width() || startPosition.y >= this->height())
        {
            // We are not treating this as an error, but rather as a 'no operation'.
            return startPosition;
        }

        // Make sure to yield so that overeager watch dog timers dont fuck us up
        // (Especially common with ESP32..)
        this->m_transport->yield();

        cursor position{startPosition};

        // Draw the string. The `draw_char` method takes care to deal with
        // special characters such as line endings correctly.
        for(const char character: string)
        {
            position = this->draw_char(font, position, character, color);
        }

        return position;
    }

    rectangle graphics::measure_text(const font* font, const position& location, std::string_view string)
    {
        return this->measure_text(font, cursor{location, this->dimensions()}, string); 
    }

    rectangle graphics::measure_text(const font* font, const cursor& location, std::string_view string)
    {
        // Check font pointer..
        if(!font)
        {
            return rectangle::empty();
        }

        internal::text_measurement_state state{location};

        for(const char character: string)
        {
            this->measure_char(font, &state, character);
        }

        return state.to_rectangle();
    }

    void graphics::measure_char(const font* font, internal::text_measurement_state* state, char character)
    {
        if(!font || !state)
        {
            return;
        }

        // Handle new line..
        if(character == '\n')
        {
            // Update location, but dont change bounds for now.
            // Only printable characters do that.
            state->location.next_line(font->y_advance);
        }
        else if(character == '\t')
        {
            // Ignore tabstops for now
            // XXX TODO figure out how to handle these
        }
        else if(character != '\r') // Carriage return gets ignored!
        {
            // Determine whether the character falls into the range
            // of glyphs that the given font can display.
            if(!font->has_character(character))
            {
                // No dice. We cant draw this character. Just skip it.
                // XXX TODO: We could maybe replace it with a '?' or something?
                return;
            }

            // Retrieve the glyph data.
            const glyph* glyph{ };
            font->glyph_for(character, &glyph);

            // We might need to do a line wrap if the current character cant fit anymore!
            if(!state->location.can_fit_glyph(glyph->x_offset + glyph->width))
            {
                state->location.next_line(font->y_advance);
            }

            // Measure the actual glyph
            std::int32_t x1 = state->location.x + glyph->x_offset;
            std::int32_t y1 = state->location.y + glyph->y_offset;
            std::int32_t x2 = x1 + glyph->width - 1;
            std::int32_t y2 = y1 + glyph->height - 1;

            if(x1 < state->minx)
            {
                state->minx = x1;
            }

            if(y1 < state->miny)
            {
                state->miny = y1;
            }

            if(x2 > state->maxx)
            {
                state->maxx = x2;
            }

            if(y2 > state->maxy)
            {
                state->maxy = y2;
            }

            state->location.next_character(glyph->x_advance);
        }
    }

    cursor graphics::draw_char(const font* font, const cursor& startPosition, char character, color color)
    {
        // Someone might have been naughty and passed us a null pointer..
        if(!font)
        {
            return startPosition;
        }

        cursor position{startPosition};

        // Handle new line.
        if(character == '\n')
        {
            position.next_line(font->y_advance);
            return position;
        }
        else if(character == '\t')
        {
            // XXX TODO: Handle tab stops correctly..
            // Maybe by advancing a configurable tab stop width multiplied
            // by the width of the 'whitespace' glyph?
            return position;
        }
        else if(character != '\r') // Carriage return would fuck us up badly.. Ignore
        {
            // Determine whether the character falls into the range
            // of glyphs that the given font can display.
            if(!font->has_character(character))
            {
                // No dice. We cant draw this character. Just skip it.
                // XXX TODO: We could maybe replace it with a '?' or something?
                return position;
            }

            // Retrieve the glyph data.
            const glyph* glyph{ };
            font->glyph_for(character, &glyph);

            // The glyph might not have a bitmap attached to it..
            // So make sure to check that
            if(glyph->width > 0 && glyph->height > 0)
            {
                // Check if we need to wrap..
                if(!position.can_fit_glyph(glyph->x_offset + glyph->width))
                {
                    position.next_line(font->y_advance);
                }

                // Actually draw the glyph
                this->draw_glyph(font, glyph, position, color);
            }

            // Either way, we will be advancing the cursors x position.
            // If the glyph did not have a bitmap attached to it, it will
            // result in a blank space.
            position.next_character(glyph->x_advance);

            return position;
        }

        return position;
    }

    void graphics::draw_glyph(const font* font, const glyph* glyph, const cursor& position, color color)
    {
        if(!font || !glyph)
        {
            return;
        }

        std::uint8_t bits{ };
        std::uint8_t bit{ };

        const auto bitmapData = font->bitmap_data_for(glyph);
        std::uint32_t bitmapDataIndex{ };
 
        for(int32_t iy = 0; iy < glyph->height; ++iy)
        {
            for(int32_t ix = 0; ix < glyph->width; ++ix)
            {
                if(!(bit++ & 7))
                {
                    bits = bitmapData[bitmapDataIndex];
                    bitmapDataIndex++;
                }

                if(bits & 0x80)
                {
                    const auto position_x = position.x + glyph->x_offset + ix;
                    const auto position_y = position.y + glyph->y_offset + iy;
                    this->draw_pixel({position_x, position_y}, color);
                }

                bits <<= 1;
            }
        }

        return;
    }

    rectangle graphics::map_rectangle_to_panel(const rectangle& rect) const
    {
        // For 0 degree rotation, we dont need to do anything.
        if(this->m_rotation == rotation::by_0deg)
        {
            return rect;
        }

        // First, rotate the top left and bottom right points
        auto topLeft = rect.location;
        auto bottomRight = rect.bottom_right();

        topLeft = this->map_point_to_panel(topLeft);
        bottomRight = this->map_point_to_panel(bottomRight);

        // We will be needing the width for adjustments later
        const auto width = (topLeft.x - bottomRight.x);
        const auto height = (topLeft.y - bottomRight.y);
        

        // Now we have to adjust the points depending on the rotation, since
        // the relative location of the points might change (and thus
        // the top left point might not be the top left one anymore, potentially)
        switch(this->m_rotation)
        {
            case rotation::by_90deg:
                // After rotation, topLeft is actually topRight,
                // and bottomRight is actually bottomLeft
                topLeft.x = topLeft.x - width;
                bottomRight.x = bottomRight.x + width;
                break;

            case rotation::by_180deg:
                // After rotation, topLeft is actually bottomRight and vise
                // versa
                std::swap(topLeft, bottomRight);
                break;

            case rotation::by_270deg:
                // After rotation, topLeft is actually bottomRight, and
                // bottomRight is actually topRight
                topLeft.y = topLeft.y - height;
                bottomRight.y = bottomRight.y + height;
                break;

            default:
                break;
        }

        return rectangle::from_points(topLeft, bottomRight);
    }

    position graphics::map_point_to_panel(const position& point) const
    {

        std::int32_t x = point.x;
        std::int32_t y = point.y;

        switch(this->m_rotation)
        {
            case epd::rotation::by_90deg:
                x = this->m_panel->width() - 1 - point.y;
                y = point.x;
                break;

            case epd::rotation::by_180deg:
                x = this->m_panel->width() - 1 - point.x;
                y = this->m_panel->height() - 1 - point.y;
                break;

            case epd::rotation::by_270deg:
                x = point.y;
                y = this->m_panel->height() - 1 - point.x;
                break;

            default:
                break;
        }

        return { x, y };
    }
}