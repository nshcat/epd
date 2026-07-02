#include <esp_err.h>
#include <esp_log.h>
#include "epd_color.hxx"
#include "epd_esp32_transport.hxx"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"

#include <epd_esp32.hxx>
#include <panels/epd_panel_GDEY037T03.hxx>


#define EPAPER_HOST SPI2_HOST
#define EPAPER_PIN_NUM_MISO GPIO_NUM_19
#define EPAPER_PIN_NUM_MOSI GPIO_NUM_23
#define EPAPER_PIN_NUM_CLK  GPIO_NUM_22
#define EPAPER_PIN_NUM_CS   GPIO_NUM_27
#define EPAPER_PIN_NUM_DC  GPIO_NUM_14
#define EPAPER_PIN_NUM_RST  GPIO_NUM_12
#define EPAPER_PIN_NUM_BUSY GPIO_NUM_13


static const char *TAG = "epd_example";

extern "C"
void app_main(void)
{
    spi_host_device_t spi_host = EPAPER_HOST;

    spi_bus_config_t buscfg = {
        .mosi_io_num = (gpio_num_t)EPAPER_PIN_NUM_MOSI,
        .miso_io_num = (gpio_num_t)EPAPER_PIN_NUM_MISO,
        .sclk_io_num = (gpio_num_t)EPAPER_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 9000,
		.flags = SPICOMMON_BUSFLAG_MASTER
    };

    ESP_LOGI(TAG, "Initializing SPI");
    esp_err_t status = spi_bus_initialize(spi_host, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(status);

    epd::esp32::pinmap epaperPins{
        .busy_pin = EPAPER_PIN_NUM_BUSY,
        .reset_pin = EPAPER_PIN_NUM_RST,
        .dc_pin = EPAPER_PIN_NUM_DC,
        .cs_pin = EPAPER_PIN_NUM_CS
    };
    epd::esp32::transport transport{epaperPins, EPAPER_HOST};

    epd::panels::GDEY037T03 panel{&transport};

    status = panel.initialize();
    if(status != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to intialize display");
    }


    ESP_LOGI(TAG, "Start idling");
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
