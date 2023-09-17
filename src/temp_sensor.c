#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>

#include "temp_sensor.h"
#include "prj_config.h"
#include "circular_array.h"
#include "hw_mcp9808.h"

#define SEMI_WAIT_TIME (100 / portTICK_PERIOD_MS)

#define LOG_TAG "i2c"

static SemaphoreHandle_t s_value_mutex = NULL;

int32_t s_last_value = TPS_NO_VALUE;
uint8_t s_last_error = 0x00;

size_t s_on_deck_hist_idx = 0;
int32_t s_history_values[TPS_HIST_READ_SIZE];

static void update_values(int32_t faren_temp, uint8_t error);

/**
 * Initialize the temperature sensor.
*/
int tps_init()
{
    // Create a Mutex to lock the "value" section of this module.
    s_value_mutex = xSemaphoreCreateMutex();
    if (s_value_mutex == NULL)
    {
        ESP_LOGI(LOG_TAG, "Failed to create temp sensor mutex");
        return 1;
    }

    // Initialize last readings to "invalid" values.
    for(int i = 0; i <  TPS_HIST_READ_SIZE; ++i)
    {
        s_history_values[i] = TPS_NO_VALUE;
    }

    return TPS_OK;
}

/**
 * Task loop for polling the temperature sensor.
*/
void tps_task(void* params)
{
    // TODO - demo loop. Break out I2C sensor and maths.
    for(;;)
    {
        TickType_t loop_start = xTaskGetTickCount();

        // Read the sensor.
        int16_t sensor_value = 0;
        int mcp_rc = hw_mcp9808_read_temp(&sensor_value);

        if (mcp_rc == HW_MCP9808_OK)
        {
            update_values(sensor_value, TPS_TEMP_OK);
        }
        else
        {
            // Sensor fail mode.
            update_values(TPS_NO_VALUE, TPS_TEMP_OK);
        }

        // Figure out how much time has passed to change how long to delay between readings.
        TickType_t loop_end = xTaskGetTickCount();
        TickType_t elapsed = loop_end - loop_start;
        // Need to be mindful that the ticks are in RTOS's tick rate, not milliseconds.
        TickType_t delay_amt = (TPS_POLL_RATE_MS / portTICK_PERIOD_MS) - elapsed;
        vTaskDelay(delay_amt);
    }
}

/**
 * Get last temperature reading values. Thread safe.
*/
int tps_get_last(int32_t* last_value, uint8_t* last_error)
{
    // Must take lock to read.
    BaseType_t take_success = xSemaphoreTake(s_value_mutex, SEMI_WAIT_TIME);
    if (take_success == pdFALSE)
    {
        *last_value = TPS_NO_VALUE;
        *last_error = 10; // TODO - meaningful value!.
        return TPS_FAIL;
    }

    *last_value = s_last_value;
    *last_error = s_last_error;

    // Must give back lock!
    xSemaphoreGive(s_value_mutex);

    return TPS_OK;
}

int tps_get_hist_values(int32_t* hist_array, ssize_t size)
{
    // History size must be smaller than integer for this code to work!
    static_assert(TPS_HIST_READ_SIZE < INT_MAX);

    if (hist_array == NULL || size <= 0)
    {
        return 0;
    }

    // Lock data while reading.
    BaseType_t take_success = xSemaphoreTake(s_value_mutex, SEMI_WAIT_TIME);
    if (take_success == pdFALSE)
    {
        return 0;
    }

    // Reading from a circular buffer. Want to give values from most recent to oldest. Note that the "current index" is
    // pointing to the oldest value at this moment.
    int start_idx = CA_PREV_IDX(s_on_deck_hist_idx, TPS_HIST_READ_SIZE);
    int idx = start_idx;
    const int32_t* pend = hist_array + size;
    int32_t* p = hist_array;

    do
    {
        // Stop once an invalid value is found. This is for the "just started up" case where there is not enough
        // history.
        if (s_history_values[idx] == TPS_NO_VALUE)
        {
            break;
        }

        *p = s_history_values[idx];

        ++p;
        idx = CA_PREV_IDX(idx, TPS_HIST_READ_SIZE);

    } while (idx != start_idx && p < pend);
    
    // Must free lock!
    xSemaphoreGive(s_value_mutex);

    // return the count of items copied.
    ssize_t count = (ssize_t)(p - hist_array);
    return count;
}

/**
 * Update last temperature reading values. Thread safe.
*/
static void update_values(int32_t faren_temp, uint8_t error)
{
    // Must lock data to write.
    BaseType_t take_success = xSemaphoreTake(s_value_mutex, SEMI_WAIT_TIME);
    if (take_success == pdFALSE)
    {
        return;
    }

    if (error == 0)
    {
        s_last_error = 0;
        s_last_value = faren_temp;

        // Set historical. The s_on_deck_hist_idx will point to the index we want to update.
        s_history_values[s_on_deck_hist_idx] = s_last_value;
        // Increment the index, wrapping to the front to create a circular array.
        s_on_deck_hist_idx = CA_NEXT_IDX(s_on_deck_hist_idx, TPS_HIST_READ_SIZE);
    }
    else
    {
        s_last_error = error;
        s_last_value = TPS_NO_VALUE;
    }

    // Must free lock!
    xSemaphoreGive(s_value_mutex);
}
