#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsonc.h"

char *escape_string(const char *str){
    char *escaped = calloc(strlen(str)*2+1, sizeof(char));
    if(!escaped) return NULL;

    char *p = escaped;

    while(*str){
        if(*str == '\"' || *str == '\\'){
            *p++ = '\\';
        }
        *p++ = *str++;
    }
    *p = '\0';

    return escaped;
}

int ensure_buffer_size(char **buffer, size_t *max_size, size_t required_space){
    if(!buffer) return -1;
    if(required_space > *max_size){
        *max_size = required_space;
        *buffer = realloc(*buffer, *max_size);
        if(!*buffer) perror("ensure_buffer_size failed.");
    }
    return 0;
}

void json_to_string(const Json *json, char **buffer, size_t *pos, size_t *max_size){
    if(!json) return;
    switch (json->type) {
        case JSON_NULL: {
            ensure_buffer_size(buffer, max_size, *pos + 4);
            *pos += snprintf(*buffer + *pos, *max_size - *pos, "null");
            break;
        }

        case JSON_TRUE: {
            ensure_buffer_size(buffer, max_size, *pos + 4);
            *pos += snprintf(*buffer + *pos, *max_size - *pos, "true");
            break;
        }

        case JSON_FALSE: {
            ensure_buffer_size(buffer, max_size, *pos + 5);
            *pos += snprintf(*buffer + *pos, *max_size - *pos, "false");
            break;
        }

        case JSON_NUMBER: {
            ensure_buffer_size(buffer, max_size, *pos + 32);
            *pos += snprintf(*buffer + *pos, *max_size - *pos, "%g", json->value.number);
            break;
        }

        case JSON_STRING: {
            char *escaped = escape_string(json->value.string);
            size_t escaped_len = strlen(escaped) + 2;
            ensure_buffer_size(buffer, max_size, *pos + escaped_len);
            *pos += snprintf(*buffer + *pos, *max_size - *pos, "\"%s\"", escaped);
            free(escaped);
            break;
        }

        case JSON_ARRAY: {
            ensure_buffer_size(buffer, max_size, *pos + 2);
            (*buffer)[(*pos)++] = '[';
            Json *element = json->value.array.element;
            while(element){
                json_to_string(element, buffer, pos, max_size);

                element = element->value.array.next;
                if(element){
                    ensure_buffer_size(buffer, max_size, *pos + 1);
                    (*buffer)[(*pos)++] = ',';
                }
            }

            (*buffer)[(*pos)++] = ']';
            break;
        }

        case JSON_OBJECT: {
            ensure_buffer_size(buffer, max_size, *pos + 2);
            (*buffer)[(*pos)++] = '{';
            Json *current = json->value.object.next;
            while(current){
                char *escaped_key = escape_string(current->value.object.key);
                size_t escaped_len = strlen(escaped_key) + 4;
                ensure_buffer_size(buffer, max_size, *pos + escaped_len);
                *pos += snprintf(*buffer + *pos, *max_size - *pos, "\"%s\": ", escaped_key);
                free(escaped_key);
                json_to_string(current->value.object.value, buffer, pos, max_size);

                current = current->value.object.next;
                if(current){
                    ensure_buffer_size(buffer, max_size, *pos + 1);
                    (*buffer)[(*pos)++] = ',';
                }

            }
            (*buffer)[(*pos)++] = '}';
            break;
        }
    }
}
