#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "json_utils.h"

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

const char *skip_whitespace(const char *str){
    while(*str && isspace(*str)){
        str++;
    }
    return str;
}

int ensure_buffer_size(char **buffer, size_t *max_size, size_t required_space){
    if(!buffer || !*buffer) {
        perror("Invalid buffer");
        return -1;
    }
    if(required_space >= *max_size){
        *max_size = required_space + 1;
        char *new_buffer = realloc(*buffer, *max_size);
        if(!new_buffer){
            perror("ensure_buffer_size failed.");
            return -2;
        }
        *buffer = new_buffer;
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
            ensure_buffer_size(buffer, max_size, *pos + 1);
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

            ensure_buffer_size(buffer, max_size, *pos + 1);
            (*buffer)[(*pos)++] = ']';
            break;
        }

        case JSON_OBJECT: {
            ensure_buffer_size(buffer, max_size, *pos + 1);
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
            ensure_buffer_size(buffer, max_size, *pos + 1);
            (*buffer)[(*pos)++] = '}';
            break;
        }
    }
}

double parse_number(const char **str){
    char *endptr;
    double num = strtod(*str, &endptr);
    if(endptr == *str) return 0;
    *str = endptr;
    return num;
}

Json *parse_value(const char **str){
    *str = skip_whitespace(*str);

    switch(**str){
        case '{': return parse_object(str);
        case '[': return parse_array(str);
        case '\"': {
            char *parsed_str = parse_string(str);
            if(!parsed_str) return NULL;
            Json *json_string = json_create_string(parsed_str);
            free(parsed_str);
            return json_string;
        }
        case 't': {
            if(strncmp(*str, "true", 4) == 0){
                *str += 4;
                return json_create_true();
            }
            break;
        }
        case 'f': {
            if(strncmp(*str, "false", 5) == 0){
                *str += 5;
                return json_create_false();
            }
            break;
        }
        case 'n': {
            if(strncmp(*str, "null", 4) == 0){
                *str += 4;
                return json_create_null();
            }
            break;
        }
        default:
            if(isdigit(**str) || **str == '-'){
                return json_create_number(parse_number(str));
            }
            break;
    }
    return NULL;
}

char *parse_string(const char **str){
    (*str)++;

    const char *start = *str;

    size_t length = 0;
    while(**str && **str != '\"'){
        if(**str == '\\') (*str)++;
        length++;
        (*str)++;
    }
    (*str)++;

    char *string = malloc(length + 1);
    if(!string) return NULL;

    const char *src = start;
    char *dest = string;
    while(*src && *src != '\"'){
        if(*src == '\\') src++;
        *dest++ = *src++;
    }
    *dest = '\0';
    return string;
}

Json *parse_object(const char **str){
    Json *object = json_create_object();
    if(!object) return NULL;

    *str = skip_whitespace(*str);
    if(**str != '{') {
        json_free(object);
        return NULL;
    }

    (*str)++;
    while(**str && **str != '}'){
        *str = skip_whitespace(*str);

        if(**str != '\"') {
            json_free(object);
            return NULL;
        }

        char *key = parse_string(str);
        if(!key){
            json_free(object);
            return NULL;
        }

        *str = skip_whitespace(*str);
        if(**str != ':') {
            free(key);
            json_free(object);
            return NULL;
        }
        (*str)++;


        *str = skip_whitespace(*str);
        Json *value = parse_value(str);
        if(!value) {
            free(key);
            json_free(object);
            return NULL;
        }

        json_object_add(object, key, value);
        free(key);

        *str = skip_whitespace(*str);
        if(**str == ',') {(*str)++;}
        else if (**str != '}'){
            json_free(object);
            return NULL;
        }
    }

    return object;
}

Json *parse_array(const char **str){
    Json *array = json_create_array();
    if(!array) return NULL;

    *str = skip_whitespace(*str);
    if(**str != '[') {
        json_free(array);
        return NULL;
    }
    (*str)++;

    while(**str && **str != ']'){
        *str = skip_whitespace(*str);

        Json *element = parse_value(str);
        if(!element) {
            json_free(array);
            return NULL;
        }

        json_array_add(array, element);

        *str = skip_whitespace(*str);
        if(**str == ',') (*str)++;
        else if(**str != ']'){
            json_free(array);
            return NULL;
        }
    }
    (*str)++;
    return array;
}
