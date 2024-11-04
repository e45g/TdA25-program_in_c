#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "json_utils.h"

static inline const char *skip_whitespace(const char *str){
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
        *max_size = required_space * 2;
        char *new_buffer = realloc(*buffer, *max_size);
        if(!new_buffer){
            perror("ensure_buffer_size failed.");
            return -2;
        }
        *buffer = new_buffer;
    }
    return 0;
}

void append_literal(char **buffer, size_t *pos, size_t *max_size, const char *literal){
    size_t len = strlen(literal);
    ensure_buffer_size(buffer, max_size, *pos + len);
    memcpy(*buffer + *pos, literal, len);
    *pos += len;
}

void escape_and_append_string(char **buffer, size_t *pos, size_t *max_size, const char *str){
    ensure_buffer_size(buffer, max_size, *pos + 2);
    (*buffer)[(*pos)++] = '"';
    while(*str){
        if(*str == '"' || *str == '\\'){
            ensure_buffer_size(buffer, max_size, *pos + 2);
            (*buffer)[(*pos)++] = '\\';
            (*buffer)[(*pos)++] = *str++;
        }
        else {
            ensure_buffer_size(buffer, max_size, *pos + 1);
            (*buffer)[(*pos)++] = *str++;
        }
    }
    ensure_buffer_size(buffer, max_size, *pos + 1);
    (*buffer)[(*pos)++] = '"';
}

void json_to_string(const Json *json, char **buffer, size_t *pos, size_t *max_size){
    if(!json) return;
    switch (json->type) {
        case JSON_NULL: {
            append_literal(buffer, pos, max_size, "null");
            break;
        }

        case JSON_TRUE: {
            append_literal(buffer, pos, max_size, "true");
            break;
        }

        case JSON_FALSE: {
            append_literal(buffer, pos, max_size, "false");
            break;
        }

        case JSON_NUMBER: {
            ensure_buffer_size(buffer, max_size, *pos + 32);
            *pos += snprintf(*buffer + *pos, *max_size - *pos, "%g", json->value.number);
            break;
        }

        case JSON_STRING: {
            escape_and_append_string(buffer, pos, max_size, json->value.string);
            break;
        }

        case JSON_ARRAY: {
            ensure_buffer_size(buffer, max_size, *pos + 1);
            (*buffer)[(*pos)++] = '[';

            for(size_t i = 0; i < json->value.array.size; ++i){
                json_to_string(json->value.array.elements[i], buffer, pos, max_size);
                if(i < json->value.array.size - 1){
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
                escape_and_append_string(buffer, pos, max_size, current->value.object.key);
                append_literal(buffer, pos, max_size, ": ");
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
    Json *array = json_create_array(0);
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
