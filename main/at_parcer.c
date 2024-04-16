#include "at_parcer.h"

//АТ парсер говно лучше так, но это не точно

void write_uart(int uart_num, char* inp, size_t len){
    uart_write_bytes(uart_num, inp, len);
}

bool parse_command(char* input, int uart_num){
    if(strncmp("AT+", input, strlen("AT+")) != 0){
        return false;
    }
    char cmd[17];
    char data[65];
    
    if(sscanf(input, "%[^=]=%s", cmd, data) == 2){
        strcat(cmd, "=");
    }

    if(strcmp(cmd, "AT+SSIDPASS=") == 0){
        char ssid[33], pass[33];
        if(sscanf(data, "%[^,],%[^ ]", ssid, pass) == 1){
            strcpy(pass, "");
        }
        connect_ssid_pass(ssid, pass);
    }else if(strcmp(cmd, "AT+WSROOM=") == 0){
        char room[33];
        sscanf(data, "%s", room);
        ws_room_set(room);
    }else if(strcmp(cmd, "AT+SERVADRR=") == 0){
        char server_addr[65];
        sscanf(data, "%s", server_addr);
        ws_addr_set(server_addr);
    }else if(strcmp(cmd, "AT+LED=") == 0){
        int brightness;
        sscanf(data, "%d", &brightness);
        set_brightness(brightness);
    }else if(strcmp(cmd, "AT+NETCONN") == 0){
        char out[12];
        sprintf(out, "+NETCONN:%d\n", network_working());
        write_uart(uart_num, out, sizeof(out));
    }else if(strcmp(cmd, "AT+RESET") == 0){
        esp_restart();
    }else{
        return false;
    }
    return true;
}