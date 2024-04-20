#include "network.h"

wifi_config_t wifi_config = {
    .sta = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
        .failure_retry_cnt = WIFI_ALL_CHANNEL_SCAN,
    },
};

esp_websocket_client_config_t ws_cfg = {
    .uri = WS_SERVER_ADDRESS,
    .reconnect_timeout_ms = 1000,
};

esp_websocket_client_handle_t client;

char ws_room[17] = "";
char ws_addr[65] = "";

void (*ws_data_callback)(char*, size_t);

void register_data_callback(void(*callback)(char*, size_t)){
    ws_data_callback = callback;
}

void send_ws_data(char* out, size_t len){
    esp_websocket_client_send_text(client, out, len, portMAX_DELAY);
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_CONNECTED");
        char output1[32] = "connect ";
        strcat(output1, ws_room);
        esp_websocket_client_send_text(client, output1, strlen(output1), portMAX_DELAY);
        set_led_state(CONNECTED);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_DISCONNECTED");
        set_led_state(WS_ERROR);
        break;
    case WEBSOCKET_EVENT_DATA:
        if(data->op_code == 1 && ws_data_callback != NULL){
            ws_data_callback((char *)data->data_ptr, data->data_len);
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI("ws", "WEBSOCKET_EVENT_ERROR");
        set_led_state(WS_ERROR);
        break;
    }
}

void wifi_disconnect_callback(){
    set_led_state(WIFI_ERROR);
    esp_wifi_connect();
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

bool network_working(){
    return esp_websocket_client_is_connected(client);
}

void ws_addr_set(char* new_ws_addr){
    strcpy(ws_addr, new_ws_addr);
    set_val("ws_addr", (char*)new_ws_addr);
    esp_websocket_client_stop(client);
    esp_websocket_client_set_uri(client, new_ws_addr);
    esp_websocket_client_start(client);
}

void ws_room_set(char* room){
    set_val("ws_room", (char*)room);
    memcpy(ws_room, room, strlen(room));
    if(esp_websocket_client_is_connected(client)){
        char output1[32] = "connect ";
        strcat(output1, room);
        esp_websocket_client_send_text(client, output1, strlen(output1), portMAX_DELAY);
    }
}

void load_from_nvs(){
    char ssid[32], pass[32];
    get_val("wifi_ssid", (char*)&ssid, sizeof(ssid));
    get_val("wifi_pass", (char*)&pass, sizeof(pass));
    get_val("ws_room", (char*)&ws_room, sizeof(ws_room));
    get_val("ws_addr", (char*)ws_addr, sizeof(ws_addr));
    if(strcmp(ssid, "") != 0){
        memcpy(wifi_config.sta.ssid, ssid, sizeof(ssid));
        memcpy(wifi_config.sta.password, pass, sizeof(pass));
    }
    if(strcmp(ws_addr, "") != 0){
        ws_cfg.uri = ws_addr;
    }
}

void init_network(){
    set_led_state(CONNECTING_WIFI);
    load_from_nvs();
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
    ESP_LOGI("wifi", "%s, %s", wifi_config.sta.ssid, wifi_config.sta.password);
}

void connect_ssid_pass(char* ssid_inp, char* pass_inp){
    char ssid[32], pass[32];
    memcpy(ssid, ssid_inp, sizeof(ssid));
    memcpy(pass, pass_inp, sizeof(ssid));
    esp_wifi_disconnect();
    set_val("wifi_ssid", (char*)ssid);
    set_val("wifi_pass", (char*)pass);
    memcpy(wifi_config.sta.ssid, ssid, sizeof(ssid));
    memcpy(wifi_config.sta.password, pass, sizeof(pass));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();
}

void start_ota(char* url){
    set_led_state(OTA_UPDATE);
    esp_http_client_config_t config = {
        .url = url,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    ESP_LOGI("ota", "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI("ota", "OTA Succeed, Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE("ota", "Firmware upgrade failed");
        set_led_state(OTA_ERROR);
    }
}