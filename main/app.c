#include "app.h"

uart_config_t uart_config = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

void uart_init(){
    ESP_ERROR_CHECK(uart_driver_install(HW_UART_NUM, 2048, 2048, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(HW_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(HW_UART_NUM, UART_RX, UART_TX, -1, -1));
    uart_write_bytes(HW_UART_NUM, "+UART_STARTED:1\n", sizeof("+UART_STARTED:1\n"));
}

void websocket_listener(char* inp, size_t len){
    ESP_LOGI("ws", "%.*s\n", len, inp);
    uart_write_bytes(HW_UART_NUM, inp, len);
    uart_write_bytes(HW_UART_NUM, "\n", 1);
}

void try_read_line(char* line, int max_len) {
	int read, pos = 0;
    char symb;
	while(1) {
		read = uart_read_bytes(HW_UART_NUM, &symb, 1, portMAX_DELAY);
		if (read == 1) {
            line[pos] = symb;
			if (symb == '\n') {
				line[pos] = 0;
				return;
			}
            if(max_len <= pos-2){
                line[pos] = 0;
                return;
            }
            if(symb != '\r'){
                pos++;
            }
		}
	}
}

void app_worker(){
    char inp[512];
    while(true){
        try_read_line(inp, sizeof(inp));
        if(!parse_command(inp, HW_UART_NUM)){
            send_ws_data(inp, strlen(inp));
        }
        ESP_LOGI("uart", "%.*s\n", strlen(inp), inp);
    }
}

void app_init(){
    uart_init();
    register_data_callback(websocket_listener);
    xTaskCreate(app_worker, "app_worker", 4096, NULL, tskIDLE_PRIORITY+1, NULL);
}