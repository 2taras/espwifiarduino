#ifndef STORAGE_H_
#define STORAGE_H_

#include "nvs.h"
#include <string.h>

void storage_init();
void get_val(char* key, char* out, size_t str_len);
void set_val(char* key, char* val);

#endif