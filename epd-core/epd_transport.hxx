#pragma once

#include <cstddef>
#include <cstdint>

#include "epd_error.hxx"

namespace epd
{
    enum class pin_state
    {
        low = 0,
        high = 1
    };

    // Abstraction over device-specific IO
    class transport
    {
    public:
        transport() = default;
        virtual ~transport() = default;

        transport(const transport&) = delete;
        transport& operator=(const transport&) = delete;

        transport(transport&&) = default;
        transport& operator=(transport&&) = default;

    public: 
        virtual error_t initialize() = 0;
        virtual error_t deinitialize() = 0;
        bool is_initialized() const;

        virtual error_t delay(std::uint32_t milliseconds) = 0;
        virtual error_t yield() = 0;

        virtual bool has_hw_reset() const = 0;
        virtual bool has_busy_pin() const = 0;
        virtual error_t get_busy_pin(pin_state* state) = 0;
        virtual error_t set_dc_pin(pin_state state) = 0;
        virtual error_t set_cs_pin(pin_state state) = 0;
        virtual error_t set_reset_pin(pin_state state) = 0;

        error_t send_command(std::uint8_t command);
        virtual error_t send_command(std::uint8_t command, std::size_t dataLength, const std::uint8_t* data) = 0;

        virtual error_t allocate_buffer(std::size_t bufferSize, std::uint8_t** buffer) = 0;
        virtual error_t free_buffer(std::uint8_t* buffer) = 0;

    protected:
        void set_initialized(bool isInitialized);

    protected:
        bool m_initialized{false};
    };
}