#ifndef _WA_TEMP_SENSOR_TYPES_H_INCLUDE_GUARD
#define _WA_TEMP_SENSOR_TYPES_H_INCLUDE_GUARD

#include <inttypes.h>

#define TPS_HIST_READ_SIZE 10
#define TPS_NO_VALUE INT32_MIN

#define TPS_OK 0
#define TPS_FAIL 1

#define TPS_TEMP_OK 0
#define TPS_TEMP_FAIL 1

// Hole temperature in hundredths of degrees fahrenheit.
typedef int32_t temper_t;

#endif // _WA_TEMP_SENSOR_TYPES_H_INCLUDE_GUARD
