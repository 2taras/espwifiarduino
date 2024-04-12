idf_component_register(SRCS "main.c" "led_log.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_websocket_client led_strip esp_wifi nvs_flash)