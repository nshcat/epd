#include "epd_error.hxx"
#include <epd_transport.hxx>

namespace epd
{
    error_t transport::send_command(std::uint8_t command)
    {
        return this->send_command(command, 0, nullptr);
    }

    void transport::set_initialized(bool isInitialized)
    {
        this->m_initialized = isInitialized;
    }

    bool transport::is_initialized() const
    {
        return this->m_initialized;
    }
}