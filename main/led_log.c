#include "led_log.h"
#include "freertos/semphr.h"

volatile static int led_state = JUST_STARTED;
volatile static int led_brightness = 255;

SemaphoreHandle_t led_sem = NULL;

led_strip_handle_t led_strip;

led_strip_config_t strip_config = {
    .strip_gpio_num = 25, // The GPIO that connected to the LED strip's data line
    .max_leds = 1, // The number of LEDs in the strip,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
    .led_model = LED_MODEL_WS2812, // LED strip model
    .flags.invert_out = false, // whether to invert the output signal (useful when your hardware has a level inverter)
};

led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
    .resolution_hz = 10 * 1000 * 1000, // 10MHz
    .flags.with_dma = false, // whether to enable the DMA feature
};

void reg_led_task(){
    led_sem = xSemaphoreCreateMutex();
    led_init();
    xTaskCreate(led_worker, "led_worker", 2048, NULL, tskIDLE_PRIORITY+1, NULL);
}

void led_init(){
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

void led_set_color(int color, long time, float blink_int, int brigness){
    int val = cos(time*blink_int)*(brigness/2) + brigness/2;
    ESP_ERROR_CHECK(led_strip_set_pixel_hsv(led_strip, 0, color, 255, val));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

void led_worker(){
    while (true){
        xSemaphoreTake(led_sem, portMAX_DELAY);
        int led_now_state = led_state;
        int brigness_now = led_brightness;
        xSemaphoreGive(led_sem);
        long time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        int color = 0;
        float blink_k = 0.005;
        switch (led_now_state){
            case JUST_STARTED:
                color = 50;
                break;
            case CONNECTING_WIFI:
                color = 70;
                break;
            case CONNECTING_WS:
                color = 90;
                break;
            case CONNECTED:
                color = 90;
                blink_k = 0;
                break;
            case WIFI_ERROR:
                color = 120;
                break;
            case WS_ERROR:
                color = 150;
                break;
            case OTA_UPDATE:
                color = 200;
                blink_k = 0.01;
                break;
            case OTA_ERROR:
                color = 220;
                break;
        }
        led_set_color(color, time_ms, blink_k, brigness_now);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool set_led_state(int new_state){
    if(led_sem != NULL && xSemaphoreTake(led_sem, pdMS_TO_TICKS(100)) == pdTRUE){
        led_state = new_state;
        xSemaphoreGive(led_sem);
        return true;
    }
    return false;
}

bool set_brightness(int brightness_new){
    if(led_sem != NULL && xSemaphoreTake(led_sem, pdMS_TO_TICKS(100)) == pdTRUE){
        led_brightness = brightness_new;
        xSemaphoreGive(led_sem);
        return true;
    }
    return false;
}