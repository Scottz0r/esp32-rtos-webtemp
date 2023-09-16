/**
 * This module defines the hardware user interface (blinking lights).
*/
#ifndef _WA_HARDWARE_UI_H_INCLUDE_GUARD
#define _WA_HARDWARE_UI_H_INCLUDE_GUARD

#define HUI_OK 0
#define HUI_FAIL 1

int hui_init();

void hui_main_task(void* params);

#endif // _WA_HARDWARE_UI_H_INCLUDE_GUARD
