#ifndef _WA_TEMP_SENSOR_H_INCLUDE_GUARD
#define _WA_TEMP_SENSOR_H_INCLUDE_GUARD

#include <inttypes.h>
#include "tempr_sensor_types.h"

int tps_init();

void tps_task(void* params);

int tps_get_last(int32_t* last_value, uint8_t* last_error);

int tps_get_hist_values(int32_t* hist_array, ssize_t size);

#endif // _WA_TEMP_SENSOR_H_INCLUDE_GUARD
