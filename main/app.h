#ifndef APP_H_
#define APP_H_

#define HW_UART_NUM 1
#define UART_RX 27
#define UART_TX 26

#include "driver/uart.h"
#include "led_log.h"

#include "storage.h"
#include "network.h"
#include "at_parcer.h"

void app_init();

#endif