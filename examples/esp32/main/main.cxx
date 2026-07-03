#include <esp_err.h>
#include <esp_log.h>
#include "epd_color.hxx"
#include "epd_esp32_transport.hxx"
#include "epd_graphics.hxx"
#include "epd_types.hxx"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/i2s_hal.h"

#include <epd_esp32.hxx>
#include <panels/epd_panel_GDEY037T03.hxx>
#include <fonts/FreeSans18pt7b.h>


#define EPAPER_HOST SPI2_HOST
#define EPAPER_PIN_NUM_MISO GPIO_NUM_19
#define EPAPER_PIN_NUM_MOSI GPIO_NUM_23
#define EPAPER_PIN_NUM_CLK  GPIO_NUM_18
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

    // Setup device-specific transport implementation that will handle low-level
    // IO with the GPIO pins, the SPI host and timers.
    epd::esp32::pinmap epaperPins{
        .busy_pin = EPAPER_PIN_NUM_BUSY,
        .reset_pin = EPAPER_PIN_NUM_RST,
        .dc_pin = EPAPER_PIN_NUM_DC,
        .cs_pin = EPAPER_PIN_NUM_CS
    };
    epd::esp32::transport transport{epaperPins, EPAPER_HOST};

    // Create object for the panel we are using, based on the device-specific 
    // transport implementation.
    epd::panels::GDEY037T03 panel{&transport};

    // Panel needs to be initialized! This will automatically initialize the
    // transport for us. :)
    status = panel.initialize();
    ESP_ERROR_CHECK(status);

    // Create graphics object based on the panel we setup earlier.
    // It will provide a virtual, rotated canvas and will take care of
    // rotation all by itself. In this instance, the 90 degree rotation
    // will cause the width and height of the virtual canvas to be flipped
    // compared to the underlying panels dimensions.
    epd::graphics graphics{&panel, epd::rotation::by_90deg};

    // Clear the framebuffer with white - we will be doing a full update.
    graphics.clear(epd::color::white);
    
    // Draw some lines
    graphics.draw_hline(
        epd::position{0, (std::int32_t)graphics.height()/2}, 
        graphics.width(),
        epd::color::black
    );

    graphics.draw_line(
        epd::position{0, 0}, 
        epd::position{25, 25}, 
        epd::color::black);

    // Rectangle!
    graphics.draw_rect(
        { epd::position{25, 55}, epd::size{75, 45} }, 
        epd::color::black);

    // Filled rectangle!
    graphics.fill_rect(
        { epd::position{95, 55}, epd::size{75, 45} },
        epd::color::black
    );

    // And finally, some text.
    graphics.draw_text(
        &FreeSans18pt7b, 
        epd::position{10, 175}, 
        "Hello World! :3", 
        epd::color::black);

    // We can also draw it inverted! For now, that involved measuring the text manually.
    std::string_view invertedText{"I'm inverted!"};
    epd::position invertedTextLocation{10, 210};
    auto textRect = graphics.measure_text(&FreeSans18pt7b, invertedTextLocation, invertedText);

    // Make the bounding box a bit bigger!
    textRect.pad_horizontal(2);
    textRect.pad_vertical(2);

    // Then we need to draw the text background.
    graphics.fill_rect(textRect, epd::color::black);

    // Finally, we need to draw the text on top of it.
    graphics.draw_text(
        &FreeSans18pt7b,
        invertedTextLocation,
        invertedText,
        epd::color::white
    );

    // Then we display the updated canvas contents on the underlying panel.
    status = graphics.display();
    ESP_ERROR_CHECK(status);

    ESP_LOGI(TAG, "Start idling");
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
