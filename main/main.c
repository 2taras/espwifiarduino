#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"

#include "led_log.h"
#include "storage.h"
#include "network.h"
#include "app.h"

void nvs_prepare(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void app_main(void)
{
    reg_led_task();
    nvs_prepare();
    storage_init();
    init_network();
    app_init();
}