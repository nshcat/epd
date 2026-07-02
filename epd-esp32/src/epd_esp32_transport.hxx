#pragma once

#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../../epd-core/epd_transport.hxx"

namespace epd
{
    namespace esp32
    {
        struct pinmap
        {
        public:
            bool has_busy_pin() const;
            bool has_reset_pin() const;

        public:
            gpio_num_t busy_pin;
            gpio_num_t reset_pin;
            gpio_num_t dc_pin;
            gpio_num_t cs_pin;
        };
        
        // IO implementation for ESP32 MCUs
        class transport
            : public ::epd::transport
        {
        public:
            transport(pinmap pins, spi_host_device_t spiHost);

            virtual ~transport() = default;

            transport(const transport&) = delete;
            transport& operator=(const transport&) = delete;

            transport(transport&&) = default;
            transport& operator=(transport&&) = default;

        public:
            virtual error_t initialize() override;
            virtual error_t deinitialize() override;
            virtual error_t delay(std::uint32_t milliseconds) override;
            virtual bool has_hw_reset() const override;
            virtual bool has_busy_pin() const override;
            virtual error_t get_busy_pin(pin_state* state) override;
            virtual error_t set_dc_pin(pin_state state) override;
            virtual error_t set_cs_pin(pin_state state) override;
            virtual error_t set_reset_pin(pin_state state) override;
            virtual error_t send_command(std::uint8_t command, std::size_t dataLength, const std::uint8_t* data) override;
            virtual error_t allocate_buffer(std::size_t bufferSize, void** buffer) override;
            virtual error_t free_buffer(void* buffer) override;
        
        protected:
            pinmap m_pins;
            spi_device_handle_t m_spiDevice{ };
            spi_host_device_t m_spiHost;
        };
    }
}