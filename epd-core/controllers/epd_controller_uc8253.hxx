#pragma once

#include "../epd_error.hxx"
#include "../epd_transport.hxx"

namespace epd
{
    namespace controllers
    {
        enum class UC8253_command
            : std::uint8_t
        {
            
        };

        // Class for working with UC8253 ePaper controllers
        class UC8253
        {
            public:
                UC8253(transport* transport);

                UC8253(const UC8253&) = delete;
                UC8253& operator=(const UC8253&) = delete;

                UC8253(UC8253&&) = default;
                UC8253& operator=(UC8253&&) = default;

            public:
                error_t send_command(UC8253_command command);
                error_t send_command(UC8253_command command, std::size_t dataLength, const std::uint8_t* data);

            protected:
                transport* m_transport;
        };
    }
}