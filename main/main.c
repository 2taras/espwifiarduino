#include <stdio.h>
#include "led_strip.h"
#include "esp_websocket_client.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_log.h"

#include "secrets.h"

wifi_config_t wifi_config = {
    .sta = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
        .failure_retry_cnt = WIFI_ALL_CHANNEL_SCAN,
    },
};

const esp_websocket_client_config_t ws_cfg = {
    .uri = WS_SERVER_ADDRESS,
    .reconnect_timeout_ms = 1000,
};

esp_websocket_client_handle_t client;

#define HW_UART_NUM 1
#define UART_RX 27
#define UART_TX 26
uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_CONNECTED");
        char output1[] = "connect 1";
        esp_websocket_client_send_text(client, output1, sizeof(output1)-1, portMAX_DELAY);
        set_led_state(CONNECTED);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_DISCONNECTED");
        set_led_state(WS_ERROR);
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_DATA");
        ESP_LOGI("ws", "Received opcode=%d", data->op_code);
        ESP_LOGW("ws", "End=%d, Received=%.*s\n\n", data->fin, data->data_len, (char *)data->data_ptr);
        ESP_LOGW("ws", "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_ERROR");
        set_led_state(WS_ERROR);
        break;
    }
}

void wifi_disconnect_callback(){
    set_led_state(WIFI_ERROR);
    ESP_ERROR_CHECK(esp_wifi_connect());
    set_led_state(CONNECTING_WIFI);
}

void wifi_connected_callback(){
    set_led_state(CONNECTING_WS);
}

void got_ip_callback(){
    if(!esp_websocket_client_is_connected(client)){
        esp_websocket_client_start(client);
    }
}

void app_main(void)
{
    reg_led_task();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(uart_driver_install(HW_UART_NUM, 2048, 2048, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(HW_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(HW_UART_NUM, UART_RX, UART_TX, -1, -1));
    uart_write_bytes(HW_UART_NUM, "123456789", sizeof("123456789"));
    set_led_state(CONNECTING_WIFI);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_disconnect_callback, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_connected_callback, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_callback, NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
}