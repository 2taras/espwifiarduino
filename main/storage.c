#include "storage.h"

nvs_handle_t nvs_obj;

void storage_init(){
    ESP_ERROR_CHECK(nvs_open("app_storage", NVS_READWRITE, &nvs_obj));
}

void get_val(char* key, char* out, size_t str_len){
    if(nvs_get_str(nvs_obj, key, out, &str_len) != ESP_OK){
        out[0] = '\0';
    }
}

void set_val(char* key, char* val){
    char old_val[256];
    get_val(key, old_val, sizeof(old_val));
    if(strcmp(old_val, val) != 0){
        nvs_set_str(nvs_obj, key, val);
        nvs_commit(nvs_obj);
    }
}