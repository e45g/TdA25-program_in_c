#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "jsonc.h"

char *escape_string(const char *str);
void json_to_string(const Json *json, char **buffer, size_t *pos, size_t *max_size);

#endif
