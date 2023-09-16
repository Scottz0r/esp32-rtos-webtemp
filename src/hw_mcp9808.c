#include "hw_mcp9808.h"

#include <freertos/portmacro.h>
#include <driver/i2c.h>
#include <esp_log.h>

#include "prj_config.h"

#define LOG_TAG "mcp9808"

#define MCP9808_I2C_TIMEOUT (100 / portTICK_PERIOD_MS)
#define MCP9808_SLAVE_ADDR  0x18
#define MCP9808_TEMPR_CMD   0x05

#define MCP9808_MANU_CMD    0x06
#define MCP9808_ID_CMD      0x07

static int16_t mcp9808_convert(uint8_t msb, uint8_t lsb);

/**
 * Read the temperature from the MCP9808 sensor. Returns temperature in hundredths of degrees (xxx.xx).
 * 
 * This assumes i2c drivers were initialized.
*/
int hw_mcp9808_read_temp(int16_t* tempr)
{
    if (!tempr)
    {
        return HW_MCP9808_FAIL;
    }

    int retval = HW_MCP9808_OK;
    *tempr = HW_MCP9808_NO_VALUE;

    uint8_t write_buffer[1] = {MCP9808_TEMPR_CMD};
    uint8_t read_buffer[2] = {0, 0};

    esp_err_t i2c_rc = i2c_master_write_read_device(
        I2C_MASTER_NUM,
        MCP9808_SLAVE_ADDR,
        write_buffer,
        sizeof(write_buffer),
        read_buffer,
        sizeof(read_buffer),
        MCP9808_I2C_TIMEOUT);

    ESP_LOGI(LOG_TAG, "rc read/write rc: %u (%u, %u)", i2c_rc, read_buffer[0], read_buffer[1]);

    if (i2c_rc == ESP_OK)
    {
        *tempr = mcp9808_convert(read_buffer[0], read_buffer[1]);
        ESP_LOGI(LOG_TAG, "Temp read: %d", *tempr);
    }
    else
    {
        retval = HW_MCP9808_FAIL;
        ESP_LOGI(LOG_TAG, "Temp reading failed");
    }

    return retval;
}

/**
 * Read device information from the MCP9808 sensor.
 * 
 * This assumes i2c drivers were initialized.
*/
int hw_mcp9808_read_device_info(hw_mcp9808_dinfo* info)
{
    if (!info)
    {
        return HW_MCP9808_FAIL;
    }

    info->manufacturer_id = 0;
    info->device_id = 0;
    info->device_revision = 0;

    // Manufacturer ID.
    uint8_t write_buffer[1] = {MCP9808_MANU_CMD};
    uint8_t read_buffer[2] = {0, 0};

    esp_err_t rc = i2c_master_write_read_device(
        I2C_MASTER_NUM,
        MCP9808_SLAVE_ADDR,
        write_buffer,
        sizeof(write_buffer),
        read_buffer,
        sizeof(read_buffer),
        MCP9808_I2C_TIMEOUT);

    if (rc == ESP_OK)
    {
        // Big endian.
        info->manufacturer_id = (read_buffer[0] << 8) | read_buffer[1];
    }

    // Device ID and Revision.
    write_buffer[0] = MCP9808_ID_CMD;

    rc = i2c_master_write_read_device(
        I2C_MASTER_NUM,
        MCP9808_SLAVE_ADDR,
        write_buffer,
        sizeof(write_buffer),
        read_buffer,
        sizeof(read_buffer),
        MCP9808_I2C_TIMEOUT);

    if (rc == ESP_OK)
    {
        info->device_id = read_buffer[0];
        info->device_revision = read_buffer[1];
    }

    return HW_MCP9808_OK;
}

/**
    This method doesn't require floating point instructions, which helps with the ESP32 floating point restrictions.
*/
static int16_t mcp9808_convert(uint8_t msb, uint8_t lsb)
{
#define DEC_MULTI 100
    // Preserve the sign.
    uint8_t sign = msb & 0x10;

    // Only the low 4 bits of the msb belong to the reading.
    msb &= 0x0F;

    int16_t whole = (msb << 4 | (lsb >> 4) ) * DEC_MULTI;

    // This is if we assume there is no floating point instructions.
    int16_t frac = 0;

    frac += ((lsb >> 3) & 0x01) * 50;
    frac += ((lsb >> 2) & 0x01) * 25;
    frac += ((lsb >> 1) & 0x01) * 13;
    frac += (lsb & 0x1) * 06;

    int16_t result = whole + frac;

    // Handle 2s complement conversion.
    if (sign)
    {
        result = (256 * DEC_MULTI) - result;
    }

    // Convert Celsius to Fahrenheit.
    result = result * (9 * DEC_MULTI) / (5 * DEC_MULTI) + (32 * DEC_MULTI);

    return result;
#undef DEC_MULTI
}
