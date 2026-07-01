#include "epd_controller_uc8253.hxx"

namespace epd
{
    namespace controllers
    {
        UC8253::UC8253(transport* transport)
            : m_transport{transport}
        {

        }

        error_t UC8253::send_command(UC8253_command command)
        {
            return this->send_command(command, 0, nullptr);
        }
        
        error_t UC8253::send_command(UC8253_command command, std::size_t dataLength, const std::uint8_t* data)
        {
            return this->m_transport->send_command(static_cast<std::uint8_t>(command), dataLength, data);
        }
    }
}