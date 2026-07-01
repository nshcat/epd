#include <epd_panel.hxx>

namespace epd
{
    panel::panel(std::size_t width, std::size_t height, transport* transport)
        : m_width{width}, m_height{height}, m_transport{transport}
    {

    }
}