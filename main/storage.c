#include "storage.h"

nvs_handle_t nvs_obj;

void storage_init(){
    ESP_ERROR_CHECK(nvs_open("app_storage", NVS_READWRITE, &nvs_obj));
}

void get_val(char* key, char* out, size_t str_len){
    nvs_get_str(nvs_obj, key, out, &str_len);
}

void set_val(char* key, char* val){
    nvs_set_str(nvs_obj, key, val);
    nvs_commit(nvs_obj);
}