#include "epd_font.hxx"
#include "epd_error.hxx"
#include "epd_array_view.hxx"

namespace epd
{
    font::font( const_byte_view bitmapData,
                array_view<const glyph> glyphs,
                std::uint16_t firstChar,
                std::uint16_t lastChar,
                std::uint8_t yAdvance)
        :   first_char{firstChar},
            last_char{lastChar},
            y_advance{yAdvance},
            m_glyphBitmapData{bitmapData},
            m_glyphs{glyphs}
    {
        
    }

    bool font::has_character(char c) const
    {
        return (c >= this->first_char && c <= this->last_char);
    }

    error_t font::glyph_for(char c, const glyph** g) const
    {
        // Invalid pointer..
        if(!g)
        {
            return EPD_FAIL;
        }

        *g = nullptr;

        // Check if this font contains the given character
        if(!this->has_character(c))
        {
            return EPD_FAIL;
        }

        const auto glyphIndex = (c - this->first_char);

        *g = &this->m_glyphs[glyphIndex];

        return EPD_OK;
    }

    const_byte_view font::bitmap_data_for(const glyph* g) const
    {
        // Size of bitmap segment is total number of bytes
        // to fit all glyph pixels.
        auto segmentSize = g->width * g->height;

        if((segmentSize % 8) != 0)
        {
            segmentSize += (8 - (segmentSize % 8));
        }

        return const_byte_view{std::next(this->m_glyphBitmapData.data(), g->bitmap_offset), segmentSize};
    }
}