#ifndef NETWORK_H_
#define NETWORK_H_

#include "esp_websocket_client.h"
#include "esp_wifi.h"
#include "nvs.h"

#include "led_log.h"
#include "storage.h"
#include "secrets.h"

void init_network();
void connect_ssid_pass(char* ssid_inp, char* pass_inp);
void send_ws_data(char* out, size_t len);
void register_data_callback(void(*callback)(char*, size_t));
bool network_working();
void ws_room_set(char* room);
void ws_addr_set(char* new_ws_addr);

#endif