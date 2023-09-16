// Free RTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ESP32 headers
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <driver/i2c.h>
#include <esp_event.h>
#include <esp_netif.h>

// Project includes
#include "hardware_ui.h"
#include "prj_config.h"
#include "temp_sensor.h"
#include "webserver.h"

#define LOG_TAG "main"

#define TASK_PIN_CPU0 0
#define TASK_PIN_CPU1 1

// For I2C initialization. ESP32 specific & RTOS specific.
#define I2C_MASTER_TIMEOUT_TICKS    (I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS)
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

static void panic_state();
static esp_err_t init_esp32_i2c();

void app_main()
{
    int init_rc;

    ESP_LOGI(LOG_TAG, "Project startup");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // I2C initialization.
    ESP_ERROR_CHECK(init_esp32_i2c());

    // Must initialize these once in startup. Must do before other Wifi code!
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Project initialization.
    init_rc = hui_init();
    if (init_rc != HUI_OK)
    {
        ESP_LOGI(LOG_TAG, "HUI failed");
        panic_state();
        return;
    }

    // I2C/temperature initialization
    init_rc = tps_init();
    if (init_rc != TPS_OK)
    {
        ESP_LOGI(LOG_TAG, "Temp/i2c failed");
        panic_state();
        return;
    }

    // Initialize SoftAP
    wbs_init();

    // Task kickoff
    TaskHandle_t h_blink_task;
    xTaskCreatePinnedToCore(hui_main_task, "hui_main_task", HWUI_TASK_STACK, NULL, HWUI_TASK_PRIORITY, &h_blink_task, TASK_PIN_CPU1);

    TaskHandle_t h_tps_task;
    xTaskCreatePinnedToCore(tps_task, "tps_main_task", 4098, NULL, 2, &h_tps_task, TASK_PIN_CPU1);

    ESP_LOGI(LOG_TAG, "Initialization Complete.");
}

/**
 * Initialize the I2C driver.
*/
static esp_err_t init_esp32_i2c()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    esp_err_t rc = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (rc != ESP_OK)
    {
        return rc;
    }

    rc = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (rc != ESP_OK)
    {
        return rc;
    }

    return rc;
}

static void panic_state()
{
    ESP_LOGI(LOG_TAG, "Panic!");
}
