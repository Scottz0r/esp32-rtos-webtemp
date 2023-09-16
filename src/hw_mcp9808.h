/**
 * Module for reading MCP9808 sensor.
*/
#ifndef _WA_HW_MCP9808_H_INCLUDE_GUARD
#define _WA_HW_MCP9808_H_INCLUDE_GUARD

#include <inttypes.h>

#define HW_MCP9808_OK 0
#define HW_MCP9808_FAIL 1
#define HW_MCP9808_NO_VALUE INT16_MIN;

typedef struct hw_mcp9808_dinfo
{
    uint8_t device_id;
    uint8_t device_revision;
    uint16_t manufacturer_id;
    
} hw_mcp9808_dinfo;

int hw_mcp9808_read_temp(int16_t* tempr);

int hw_mcp9808_read_device_info(hw_mcp9808_dinfo* info);

#endif // _WA_HW_MCP9808_H_INCLUDE_GUARD
