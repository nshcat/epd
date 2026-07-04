#pragma once

#include <cstdint>

#include "epd_array_view.hxx"
#include "epd_error.hxx"

namespace epd
{
    // Struct describing a single glyph within a font.
    // Layout heavily inspired by Adafruits GFX library.
    struct glyph
    {
        std::uint16_t bitmap_offset;
        std::uint8_t width;
        std::uint8_t height;
        std::uint8_t x_advance;
        std::int8_t x_offset;
        std::int8_t y_offset;
    };

    // Font as a collection of glyphs.
    class font
    {
    public:
        font(   const_byte_view bitmapData,
                array_view<const glyph> glyphs,
                std::uint16_t firstChar,
                std::uint16_t lastChar,
                std::uint8_t yAdvance
            );

        ~font() = default;

        font(const font&) = delete;
        font& operator=(const font&) = delete;

        font(font&&) = delete;
        font& operator=(font&&) = delete;

    public:
        error_t glyph_for(char c, const glyph**) const;
        const_byte_view bitmap_data_for(const glyph* g) const;
        bool has_character(char c) const;

    public:
        std::uint16_t first_char;
        std::uint16_t last_char;
        std::uint8_t y_advance;

    protected:
        const_byte_view m_glyphBitmapData;
        array_view<const glyph> m_glyphs;
    };
}