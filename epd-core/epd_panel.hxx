#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "epd_error.hxx"
#include "epd_color.hxx"
#include "epd_transport.hxx"

namespace epd
{
    // Base class for epaper panels
    class panel
    {
    public:
        panel(std::size_t width, std::size_t height, transport* transport);

        virtual ~panel() = default;

        panel(const panel&) = delete;
        panel& operator=(const panel&) = delete;

        panel(panel&&) = default;
        panel& operator=(panel&&) = default;

    public: // Panel management
        virtual error_t initialize() = 0;
        virtual error_t sleep() = 0;
        virtual error_t refresh() = 0;
        

    public: // Panel graphics operations
        virtual error_t fill(color color) = 0;
        
    protected:
        std::size_t m_width;
        std::size_t m_height;
        transport* m_transport;
    };
}