#include <cstdint>
#include <epd_panel.hxx>

namespace epd
{
    panel::panel(std::uint32_t width, std::uint32_t height, transport* transport)
        : m_width{width}, m_height{height}, m_transport{transport}
    {

    }

    std::uint32_t panel::width() const
    {
        return this->m_width;
    }

    std::uint32_t panel::height() const
    {
        return this->m_height;
    }
}