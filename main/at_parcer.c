#include "at_parcer.h"

//АТ парсер говно лучше так, но это не точно

void write_uart(int uart_num, char* inp, size_t len){
    uart_write_bytes(uart_num, inp, len);
}

bool parse_command(char* input, int uart_num){
    if(strncmp("AT+", input, strlen("AT+")) != 0){
        return false;
    }
    if (strncmp("AT+SSIDPASS=", input, strlen("AT+SSIDPASS=")) == 0) {
        char ssid[33], pass[33];
        const char* params_start = input + strlen("AT+SSIDPASS=");
        const char* ssid_end = strchr(params_start, ',');
        if (ssid_end == NULL) {
            return false;
        }
        int ssid_length = ssid_end - params_start;
        strncpy(ssid, params_start, ssid_length);
        ssid[ssid_length] = '\0';
        const char* password_start = ssid_end + 1;
        strncpy(pass, password_start, strlen(input));
        pass[input - password_start + 1] = '\0';
        connect_ssid_pass(ssid, pass);
    } else if (strncmp("AT+NETCONN", input, strlen("AT+NETCONN")) == 0) {
        char out[strlen("+NETCONN:0")+2];
        sprintf(out, "+NETCONN:%d\n", network_working());
        write_uart(uart_num, out, sizeof(out));
    // } else if (strncmp("AT+SERVADRR", input, strlen("AT+SERVADRR")) == 0) {
    //     char ws_server[65];
    //     sscanf(input, "AT+SERVADRR=%64s", parsed->wsserveraddress);
    // } else if (strncmp("AT+WSROOM", input, strlen("AT+WSROOM")) == 0) {
    //     sscanf(input, "AT+WSROOM=%32s", parsed->wschannel);
    // } else if (strncmp("AT+LED", input, strlen("AT+LED")) == 0) {
    //     int brightness;
    //     sscanf(input, "AT+LED=%d", &parsed->ledbrightness);
    } else if (strncmp("AT+RESET", input, strlen("AT+RESET")) == 0) {
        esp_restart();
    } else {
        return false;
    }
    
    return true;
}