#ifndef AT_PARCER_H_
#define AT_PARCER_H_

#include "driver/uart.h"
#include "led_log.h"
#include "esp_system.h"

#include "storage.h"
#include "network.h"

bool parse_command(char* inp, int uart_num);

#endif