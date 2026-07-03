#include "epd_graphics.hxx"
#include "epd_error.hxx"
#include "epd_types.hxx"
#include "fonts/gfxfont.h"
#include <cstdlib>
#include <utility>

namespace epd
{
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

    void graphics::draw_pixel(position position, color color)
    {
        // Perform bounds check to avoid out-of-bounds buffer access
        if(position.x >= this->width() || position.y >= this->height())
        {
            return;
        }

        // Rotate coordinates based on current rotation mode
        std::uint32_t temp{ };
        switch(this->m_rotation)
        {
            case epd::rotation::by_90deg:
                temp = position.x;
                position.x = this->m_panel->width() - 1 - position.y;
                position.y = temp;
                break;

            case epd::rotation::by_180deg:
                position.x = this->m_panel->width() - 1 - position.x;
                position.y = this->m_panel->height() - 1 - position.y;
                break;

            case epd::rotation::by_270deg:
                temp = position.x;
                position.x = position.y;
                position.y = this->m_panel->height() - 1 - temp;
                break;

            default:
                break;
        }

        // Write to panel framebuffer
        this->m_panel->set_pixel(position, color);
    }

    void graphics::draw_hline(position position, std::int32_t width, color color)
    {
        epd::position to{ position.x + static_cast<std::int32_t>(width) - 1, position.y };
        this->draw_line(position, to, color);
    }

    void graphics::draw_vline(position position, std::int32_t height, color color)
    {
        epd::position to{ position.x, position.y + static_cast<std::int32_t>(height) - 1 };
        this->draw_line(position, to, color);
    }

    void graphics::draw_line(position from, position to, color color)
    {
        // Make sure to yield so that overeager watch dog timers dont fuck us up
        // (Especially common with ESP32..)
        this->m_transport->yield();

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

    void graphics::draw_rect(position topLeft, size dimensions, color color)
    {
        this->draw_hline(topLeft, dimensions.width, color);
        this->draw_hline(epd::position{topLeft.x, topLeft.y + (std::int32_t)dimensions.height - 1}, dimensions.width, color);

        this->draw_vline(topLeft, dimensions.height, color);
        this->draw_vline(epd::position{topLeft.x + (std::int32_t)dimensions.width - 1, topLeft.y}, dimensions.height, color);
    }

    cursor graphics::draw_text(const GFXfont* font, position position, std::string_view string, color color)
    {
        return this->draw_text(font, cursor{position, this->dimensions()}, string, color);
    }

    cursor graphics::draw_text(const GFXfont* font, cursor position, std::string_view string, color color)
    {
        if(!font) // Bad pointer? For real? :'(
        {
            return position;
        }

        // Dont draw if we are already out of bounds.
        if(position.x >= this->width() || position.y >= this->height())
        {
            // We are not treating this as an error, but rather as a 'no operation'.
            return position;
        }

        // Make sure to yield so that overeager watch dog timers dont fuck us up
        // (Especially common with ESP32..)
        this->m_transport->yield();

        // Draw the string. The `draw_char` method takes care to deal with
        // special characters such as line endings correctly.
        for(const char character: string)
        {
            position = this->draw_char(font, position, character, color);
        }

        return position;
    }

    cursor graphics::draw_char(const GFXfont* font, cursor position, char character, color color)
    {
        // Someone might have been naughty and passed us a null pointer..
        if(!font)
        {
            return position;
        }

        // Handle new line.
        if(character == '\n')
        {
            position.next_line(font->yAdvance);
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
            if(character < font->first || character > font->last)
            {
                // No dice. We cant draw this character. Just skip it.
                // XXX TODO: We could maybe replace it with a '?' or something?
                return position;
            }

            // Retrieve the glyph data.
            const auto glyphIndex = (character - font->first);
            GFXglyph* glyph = (&font->glyph[glyphIndex]);

            // The glyph might not have a bitmap attached to it..
            // So make sure to check that
            if(glyph->width > 0 && glyph->height > 0)
            {
                // Check if we need to wrap..
                if(!position.can_fit_glyph(glyph->xOffset + glyph->width))
                {
                    position.next_line(font->yAdvance);
                }

                // Actually draw the glyph
                this->draw_glyph(font, glyph, position, color);
            }

            // Either way, we will be advancing the cursors x position.
            // If the glyph did not have a bitmap attached to it, it will
            // result in a blank space.
            position.next_character(glyph->xAdvance);

            return position;
        }

        return position;
    }

    void graphics::draw_glyph(const GFXfont* font, const GFXglyph* glyph, cursor position, color color)
    {
        if(!font || !glyph)
        {
            return;
        }

        std::uint8_t bits{ };
        std::uint8_t bit{ };
        std::uint32_t bitmapOffset = glyph->bitmapOffset;

        for(int32_t iy = 0; iy < glyph->height; ++iy)
        {
            for(int32_t ix = 0; ix < glyph->width; ++ix)
            {
                if(!(bit++ & 7))
                {
                    bits = font->bitmap[bitmapOffset];
                    bitmapOffset++;
                }

                if(bits & 0x80)
                {
                    const auto position_x = position.x + glyph->xOffset + ix;
                    const auto position_y = position.y + glyph->yOffset + iy;
                    this->draw_pixel({position_x, position_y}, color);
                }

                bits <<= 1;
            }
        }

        return;
    }
}