#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "hardware_ui.h"
#include "prj_config.h"

#define LOG_TAG "hui"

// Need to convert milliseconds to RTOS ticks.
#define IRL_BLINK_PERIOD_SHORT (HUI_BLINK_PERIOD_SHORT_MS / portTICK_PERIOD_MS)
#define IRL_BLINK_PERIOD_LONG (HUI_BLINK_PERIOD_LONG_MS / portTICK_PERIOD_MS)

int hui_init()
{
    ESP_LOGI(LOG_TAG, "Configuring led");
    
    gpio_config_t io_config = {};

    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = (1 << HW_PIN_BLINKY);
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;

    esp_err_t rc = gpio_config(&io_config);

    if (rc == ESP_OK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void hui_main_task(void* params)
{
    // Infinite blink loop.
    for(;;)
    {
        // Two quick blinks.
        gpio_set_level(GPIO_NUM_13, 1);
        vTaskDelay(IRL_BLINK_PERIOD_SHORT);
        gpio_set_level(GPIO_NUM_13, 0);
        vTaskDelay(IRL_BLINK_PERIOD_SHORT);
        gpio_set_level(GPIO_NUM_13, 1);
        vTaskDelay(IRL_BLINK_PERIOD_SHORT);
        gpio_set_level(GPIO_NUM_13, 0);
        vTaskDelay(IRL_BLINK_PERIOD_SHORT); // This line makes it consistent interval.

        // One long pause period between.
        vTaskDelay(IRL_BLINK_PERIOD_LONG);
    }   
}
