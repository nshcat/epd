#include "driver/spi_master.h"
#include "epd_transport.hxx"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "freertos/projdefs.h"
#include "soc/gpio_num.h"

#include "epd_esp32_transport.hxx"

#define SPI_MASTER_FREQ_4M      (80 * 1000 * 1000 / 20)   ///< 4MHz
#define SPI_MASTER_FREQ_2M      (80 * 1000 * 1000 / 40)   ///< 2MHz
#define SPI_MASTER_FREQ_1M      (80 * 1000 * 1000 / 80)   ///< 1MHz
#define SPI_MASTER_FREQ_8M      (80 * 1000 * 1000 / 10)   ///< 8MHz
#define SPI_MASTER_FREQ_500K    (80 * 1000 * 1000 / 160)  ///< 500KHz
#define SPI_MASTER_FREQ_200K    (80 * 1000 * 1000 / 400)  ///< 200KHz
#define SPI_MASTER_FREQ_20K    (80 * 1000 * 1000 / 4000)  ///< 20KHz

namespace epd
{
    namespace esp32 
    {
        static const char* TAG = "epd-esp32-transport";

        // == pinmap
        bool pinmap::has_busy_pin() const
        {
            return this->busy_pin != GPIO_NUM_NC;
        }

        bool pinmap::has_reset_pin() const
        {
            return this->reset_pin != GPIO_NUM_NC;
        }

        // == transport
        transport::transport(pinmap pins, spi_host_device_t spiHost)
            : m_pins{pins}, m_spiHost{spiHost}
        {
            
        }

        error_t transport::initialize()
        {
            if(this->is_initialized())
            {
                ESP_LOGE(TAG, "Transport is already initialized");
                return ESP_ERR_INVALID_STATE;
            }

            // = Configure GPIOs
            if(this->has_busy_pin())
            {
                gpio_set_direction(this->m_pins.busy_pin, GPIO_MODE_INPUT);
		        gpio_set_pull_mode(this->m_pins.busy_pin, GPIO_PULLUP_ONLY);
            }

            if(this->has_hw_reset())
            {
                gpio_set_direction(this->m_pins.reset_pin, GPIO_MODE_OUTPUT);
                gpio_set_level(this->m_pins.reset_pin, 1);
            }

            gpio_set_direction(this->m_pins.dc_pin, GPIO_MODE_OUTPUT);
            gpio_set_direction(this->m_pins.cs_pin, GPIO_MODE_OUTPUT);
            gpio_set_level(this->m_pins.dc_pin, 1);
            gpio_set_level(this->m_pins.cs_pin, 1);

            // = Configure SPI device
            spi_device_interface_config_t deviceConfig{ };
            deviceConfig.mode = 0;
            deviceConfig.clock_speed_hz = SPI_MASTER_FREQ_1M;
            deviceConfig.spics_io_num = -1;
            deviceConfig.queue_size = 3;

            auto result = spi_bus_add_device(this->m_spiHost, &deviceConfig, &this->m_spiDevice);
            if(result != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to add SPI device to host");
                return result;
            }

            this->set_initialized(true);
            return ESP_OK;
        }

        error_t transport::deinitialize()
        {
            if(!this->is_initialized())
            {
                ESP_LOGE(TAG, "Transport is not initialized");
                return ESP_ERR_INVALID_STATE;
            }

            spi_bus_remove_device(this->m_spiDevice);
            this->m_spiDevice = { };

            this->set_initialized(false);
            return ESP_OK;
        }

        error_t transport::delay(std::uint32_t milliseconds)
        {
            vTaskDelay(pdMS_TO_TICKS(milliseconds));

            return ESP_OK;
        }

        bool transport::has_hw_reset() const
        {
            return this->m_pins.has_reset_pin();
        }

        bool transport::has_busy_pin() const
        {
            return this->m_pins.has_busy_pin();
        }

        error_t transport::wait_for_busy()
        {
            if(!this->has_busy_pin())
            {
                ESP_LOGE(TAG, "Can't wait for busy signal: Transport has no busy GPIO pin assigned");
                return ESP_ERR_INVALID_STATE;
            }

            while(gpio_get_level(this->m_pins.busy_pin) == 1)
            {
                // We yield here briefly so the WDT does not terminate our application.
                // No busy waiting allowed! :^)
                vTaskDelay(1);
            }

            return ESP_OK;
        }

        error_t transport::set_dc_pin(pin_state state)
        {
            return gpio_set_level(this->m_pins.dc_pin, state == pin_state::high ? 1 : 0);
        }

        error_t transport::set_cs_pin(pin_state state)
        {
            return gpio_set_level(this->m_pins.cs_pin, state == pin_state::high ? 1 : 0);
        }

        error_t transport::set_reset_pin(pin_state state)
        {
            if(!this->has_hw_reset())
            {
                ESP_LOGE(TAG, "Transport has no reset GPIO pin assigned");
                return ESP_ERR_INVALID_STATE;
            }

            return gpio_set_level(this->m_pins.reset_pin, state == pin_state::high ? 1 : 0);
        }

        error_t transport::send_command(std::uint8_t command, std::size_t dataLength, const std::uint8_t* data)
        {
            // Check parameters
            if(dataLength > 0 && !data)
            {
                ESP_LOGE(TAG, "No buffer passed");
                return ESP_ERR_INVALID_ARG;
            }

            // First, send the command. For that, we need to assert the DC line.
            this->set_dc_pin(pin_state::low);
            this->set_cs_pin(pin_state::low);

            spi_transaction_t commandTransaction{ };
            commandTransaction.length = 8;
            commandTransaction.tx_buffer = &command;

            auto result = spi_device_polling_transmit(this->m_spiDevice, &commandTransaction);
            if(result != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to transmit command over SPI");
                this->set_dc_pin(pin_state::high);
                this->set_cs_pin(pin_state::high);
                return result;
            }

            this->set_dc_pin(pin_state::high);

            // Now, if needed, send the data buffer associated with the command. This requires
            // the DC line to be deasserted.
            if(dataLength > 0)
            {
                spi_transaction_t dataTransaction{ };
                dataTransaction.length = dataLength * 8;
                dataTransaction.tx_buffer = data;

                result = spi_device_polling_transmit(this->m_spiDevice, &dataTransaction);
                if(result != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to transmit data over SPI");
                    this->set_cs_pin(pin_state::high);
                    return result;
                }
            }

            // No more data to transmit. Deassert CS line.
            this->set_cs_pin(pin_state::high);
    
            return ESP_OK;
        }

        error_t transport::allocate_buffer(std::size_t bufferSize, void** buffer)
        {
            if(!buffer)
            {
                ESP_LOGE(TAG, "Invalid pointer given");
                return ESP_ERR_INVALID_ARG;
            }

            *buffer = nullptr;

            if(bufferSize == 0)
            {
                ESP_LOGE(TAG, "Requested buffer of zero size");
                return ESP_ERR_INVALID_ARG;
            }

            // We want to perform SPI transfers via DMA, so the buffers need to be allocated
            // with the DMA capability
            auto* allocatedBuffer = heap_caps_malloc(bufferSize, MALLOC_CAP_DMA);
            if(!allocatedBuffer)
            {
                ESP_LOGE(TAG, "Failed to allocate buffer of size %d", bufferSize);
                return ESP_ERR_NO_MEM;
            }

            *buffer = allocatedBuffer;
            return ESP_OK;
        }

        error_t transport::free_buffer(void* buffer)
        {
            if(!buffer)
            {
                return ESP_ERR_INVALID_ARG;
            }

            heap_caps_free(buffer);

            return ESP_OK;
        }
    }
}