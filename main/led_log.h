#ifndef LED_LOG_H_
#define LED_LOG_H_

#include "math.h"
#include "led_strip.h"
#include "esp_log.h"

#define JUST_STARTED 0
#define CONNECTING_WIFI 1
#define CONNECTING_WS 2
#define CONNECTED 3
#define WIFI_ERROR 4
#define WS_ERROR 5

void reg_led_task();
void led_init();
void led_set_color(int color, long time, float blink_int, int brigness);
void led_worker();
bool set_led_state(int new_state);
bool set_brightness(int brightness_new);

#endif