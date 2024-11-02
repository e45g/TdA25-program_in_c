#ifndef JSONC_H
#define JSONC_H

#include <stddef.h>

/*
*   TODO:
*   FIX double free problem when adding same Json* to object or array
*
*/

enum JsonType{
    JSON_NULL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

typedef struct Json{
    enum JsonType type;
    union {
        double number;
        char *string;

        struct {
            struct Json *element;
            struct Json *next;
        } array;

        struct {
            char *key;
            struct Json *value;
            struct Json *next;
        } object;

    } value;
} Json;

Json *json_create_object();
Json *json_create_array();
Json *json_create_string(const char *string);
Json *json_create_number(double number);
Json *json_parse(const char *str); // TODO

int json_array_add(Json *array, Json *value);
int json_array_remove(Json *array, size_t index); // TODO

int json_object_add(Json *object, const char *key, Json *value);
int json_object_remove(Json *object, const char *key); // TODO

char *json_print(Json *json);

void json_free(Json *json);

#endif
