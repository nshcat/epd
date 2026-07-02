#pragma once

#include "epd_color.hxx"
#include "epd_error.hxx"
#include "epd_panel.hxx"
#include "epd_transport.hxx"
#include "epd_types.hxx"

namespace epd
{
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

    public:
        void set_rotation(rotation rotation);
        epd::rotation current_rotation() const;
        std::uint32_t width() const;
        std::uint32_t height() const;

        void clear(color color);
        error_t display();

    public:
        void draw_pixel(position position, color color);
        void draw_hline(position position, std::uint32_t width, color color);
        void draw_vline(position position, std::uint32_t height, color color);
        void draw_rect(position topLeft, std::uint32_t width, std::uint32_t height, color color);
        void fill_rect(position topLeft, std::uint32_t width, std::uint32_t height, color color);
        void draw_line(position from, position to, color color);

    protected:
        panel* m_panel;
        epd::transport* m_transport; 
        std::uint32_t m_width;
        std::uint32_t m_height;
        epd::rotation m_rotation{epd::rotation::by_0deg};
    };
}