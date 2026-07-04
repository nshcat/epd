#pragma once

#include <limits>
#include <string_view>

#include "epd_color.hxx"
#include "epd_error.hxx"
#include "epd_panel.hxx"
#include "epd_transport.hxx"
#include "epd_types.hxx"
#include "epd_font.hxx"

namespace epd
{
    namespace internal
    {
        struct text_measurement_state
        {
        public:
            text_measurement_state(cursor location);

        public:
            rectangle to_rectangle() const;

        public:
            cursor location;
            position initialLocation;
            std::int32_t minx{ std::numeric_limits<std::int32_t>::max() };
            std::int32_t maxx{ -1 };
            std::int32_t miny{ std::numeric_limits<std::int32_t>::max() };
            std::int32_t maxy{ -1 };
        };
    }

    enum class rotation
        : std::uint8_t
    {
        by_0deg = 0x0,
        by_90deg = 0x1,
        by_180deg = 0x2,
        by_270deg = 0x3,
    };

    // Class for high-level graphics operations on panels
    class graphics
    {
    public:
        graphics(panel* panel, rotation rotation = rotation::by_0deg);

        graphics(const graphics&) = delete;
        graphics& operator=(const graphics&) = delete;

        graphics(graphics&&) = default;
        graphics& operator=(graphics&&) = default;

    public: // Display configuration and operations
        void set_rotation(rotation rotation);
        epd::rotation current_rotation() const;
        std::int32_t width() const;
        std::int32_t height() const;
        size dimensions() const;

        void clear(color color);
        error_t display();
        void clear_partial(const rectangle& area, color color);
        error_t display_partial(const rectangle& area);

    public: // Drawing primitives
        void draw_pixel(const position& location, color color);
        void draw_hline(const position& location, std::int32_t width, color color);
        void draw_vline(const position& location, std::int32_t height, color color);
        void draw_rect(const rectangle& rect, color color);
        void fill_rect(const rectangle& rect, color color);
        void draw_line(const position& from, const position& to, color color);

    public: // Text operations
        cursor draw_text(const font* font, const position& location, std::string_view string, color color);
        cursor draw_text(const font* font, const cursor& location, std::string_view string, color color);
        rectangle measure_text(const font* font, const position& location, std::string_view string);
        rectangle measure_text(const font* font, const cursor& location, std::string_view string);

    protected:
        cursor draw_char(const font* font, const cursor& pos, char character, color color);
        void draw_glyph(const font* font, const glyph* glyph, const cursor& position, color color);
        void measure_char(const font* font, internal::text_measurement_state* state, char character);

    public:
        rectangle map_rectangle_to_panel(const rectangle& rect) const;
        position map_point_to_panel(const position& point) const;

    protected:
        panel* m_panel;
        epd::transport* m_transport; 
        std::int32_t m_width;
        std::int32_t m_height;
        epd::rotation m_rotation{epd::rotation::by_0deg};
    };
}