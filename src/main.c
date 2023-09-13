// Free RTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

// Project includes
#include "hardware_ui.h"
#include "prj_config.h"

#define TASK_PIN_CPU0 0
#define TASK_PIN_CPU1 1

#define LOG_TAG "main"

static void panic_state();

void app_main()
{
    int init_rc;

    ESP_LOGI(LOG_TAG, "Project startup");

    // Project initialization.
    init_rc = hui_init();
    if (init_rc == 1)
    {
        panic_state();
        return;
    }

    ESP_LOGI(LOG_TAG, "Initialization Complete.");

    TaskHandle_t h_blink_task;
    xTaskCreatePinnedToCore(hui_main_task, "hui_main_task", HWUI_TASK_STACK, NULL, HWUI_TASK_PRIORITY, &h_blink_task, TASK_PIN_CPU1);

    // Debug high water marks to see how much memory these tasks need. Can't do this on the tasks
    // because LOGI needs a lot of stack.
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    BaseType_t task_hwm = uxTaskGetStackHighWaterMark(h_blink_task);
    ESP_LOGI(LOG_TAG, "Blink Task HWM: %u", task_hwm);
}

void panic_state()
{
    ESP_LOGI(LOG_TAG, "Panic!");
}
