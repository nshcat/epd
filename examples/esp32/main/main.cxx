#include <esp_err.h>
#include <esp_log.h>
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"

#include <epd_esp32.hxx>

static const char *TAG = "epd_example";

extern "C"
void app_main(void)
{
    ESP_LOGI(TAG, "Start idling");
    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
