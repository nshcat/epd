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
        
        // SPI bus speed that should be used to transmit data
        // to the epaper panel
        enum class spi_bus_speed
        {
            speed_1mhz = (80 * 1000 * 1000 / 80),
            speed_2mhz = (80 * 1000 * 1000 / 40),
            speed_4mhz = (80 * 1000 * 1000 / 20),
            speed_8mhz = SPI_MASTER_FREQ_8M,
            speed_16mhz = SPI_MASTER_FREQ_16M,
            speed_20mhz = SPI_MASTER_FREQ_20M,
            speed_26mhz = SPI_MASTER_FREQ_26M
        };

        // IO implementation for ESP32 MCUs
        class transport
            : public ::epd::transport
        {
        protected:
            // Size of data chunks that should be transmitted via SPI at once
            constexpr static std::int32_t SPI_CHUNK_SIZE = 4000;

        public:
            transport(pinmap pins, spi_host_device_t spiHost, spi_bus_speed spiSpeed = spi_bus_speed::speed_20mhz);

            virtual ~transport() = default;

            transport(const transport&) = delete;
            transport& operator=(const transport&) = delete;

            transport(transport&&) = default;
            transport& operator=(transport&&) = default;

        public:
            virtual error_t initialize() override;
            virtual error_t deinitialize() override;
            virtual error_t delay(std::uint32_t milliseconds) override;
            virtual error_t yield() override;
            virtual bool has_hw_reset() const override;
            virtual bool has_busy_pin() const override;
            virtual error_t get_busy_pin(pin_state* state) override;
            virtual error_t set_dc_pin(pin_state state) override;
            virtual error_t set_cs_pin(pin_state state) override;
            virtual error_t set_reset_pin(pin_state state) override;
            virtual error_t send_command(std::uint8_t command, std::size_t dataLength, const std::uint8_t* data) override;
            virtual error_t allocate_buffer(std::size_t bufferSize, std::uint8_t** buffer) override;
            virtual error_t free_buffer(std::uint8_t* buffer) override;
        
        protected:
            pinmap m_pins;
            spi_device_handle_t m_spiDevice{ };
            spi_host_device_t m_spiHost;
            spi_bus_speed m_spiSpeed;
        };
    }
}