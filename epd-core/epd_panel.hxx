#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "epd_error.hxx"
#include "epd_color.hxx"
#include "epd_transport.hxx"
#include "epd_types.hxx"

namespace epd
{
    // Base class for epaper panels
    class panel
    {
    public:
        panel(std::uint32_t width, std::uint32_t height, transport* transport);

        virtual ~panel() = default;

        panel(const panel&) = delete;
        panel& operator=(const panel&) = delete;

        panel(panel&&) = default;
        panel& operator=(panel&&) = default;

    public: // Panel properties
        std::uint32_t width() const;
        std::uint32_t height() const;
        epd::transport* transport() const;

    public: // Panel management
        virtual error_t initialize() = 0;
        virtual error_t sleep() = 0;
        virtual error_t refresh() = 0;
        
    public: // Panel graphics operations
        virtual error_t fill(color color) = 0;
        virtual error_t set_pixel(position pos, color color) = 0;
        
    protected:
        std::uint32_t m_width;
        std::uint32_t m_height;
        epd::transport* m_transport;
    };
}