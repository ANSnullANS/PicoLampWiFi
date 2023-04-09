#ifndef CONFIG_H
#define CONFIG_H

#include "pico/stdlib.h"
#include "pico/stdio.h"

#define VERSION_NO "1.0.0"

#define DEBUG true
//#define DEBUG_RAINBOW true

#ifndef LWIP_HTTPD_EXAMPLE_SIMPLEPOST
#define LWIP_HTTPD_EXAMPLE_SIMPLEPOST 0
#endif

#ifndef WIFI_COUNTRY
#define WIFI_COUNTRY CYW43_COUNTRY_AUSTRIA
#endif

#define TCP_PORT 80
#define BUF_SIZE 2048
#define POLL_TIME_S 5
#define LED_PIN 2
#define NUM_PIXELS 7

#define DAYLIGHT_PIN  15
#define PWR_SAVE_PIN  14

// Buffers
#define USER_PASS_BUFSIZE 16
#define ACTION_BUFSIZE 16


// Timing Variables
#define STATUS_UPDATE_INTERVAL 5000 // Interval in milliseconds
#define RAINBOW_UPDATE_INTERVAL 10000 // Interval in milliseconds

#ifndef LED_MODE_ENUM
#define LED_MODE_ENUM
enum LED_MODES {
  MODE_UNKNOWN,
  MODE_RAINBOW,
  MODE_FIXED,
  MODE_STATUS
};
#endif

#ifndef COLOR_STRUCT
#define COLOR_STRUCT
struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};
#endif

#endif // CONFIG_H