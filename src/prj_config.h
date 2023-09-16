/**
 * Project configuration constants
*/
#ifndef _WA_PRJ_CONFIG_H_INCLUDE_GUARD
#define _WA_PRJ_CONFIG_H_INCLUDE_GUARD

// Hardware I2C configuration.
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           23
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000

// Hardware User Interface
#define HUI_BLINK_PERIOD_LONG_MS 1000
#define HUI_BLINK_PERIOD_SHORT_MS 250

#define HW_PIN_BLINKY 13
#define HWUI_TASK_STACK 1024
#define HWUI_TASK_PRIORITY 1

// Interval for polling temperature sensor (milliseconds).
#define TPS_POLL_RATE_MS 60000

// Default WiFi SoftAP name and password
#define WEBS_AP_SSID "TEST AP"
#define WEBS_AP_PWD "test1234"

#endif // _WA_PRJ_CONFIG_H_INCLUDE_GUARD
