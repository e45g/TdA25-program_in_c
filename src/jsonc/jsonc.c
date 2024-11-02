#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_utils.h"
#include "jsonc.h"

Json *json_create_string(const char *string){
    Json *json = calloc(1, sizeof(Json));
    if(!json) {
        return NULL;
    }

    json->type = JSON_STRING;
    json->value.string = strdup(string);

    if(!json->value.string){
        free(json);
        return NULL;
    }

    return json;
}

Json *json_create_number(double number){
    Json *json = calloc(1, sizeof(Json));
    if(!json) {
        return NULL;
    }

    json->type = JSON_NUMBER;
    json->value.number = number;

    return json;
}

Json *json_create_object(){
    Json *json = calloc(1, sizeof(Json));
    if(!json) {
        return NULL;
    }

    json->type = JSON_OBJECT;
    json->value.object.key = NULL;
    json->value.object.value = NULL;
    json->value.object.next = NULL;

    return json;
}

Json *json_create_array(){
    Json *json = calloc(1, sizeof(Json));
    if(!json){
        return NULL;
    }

    json->type = JSON_ARRAY;
    json->value.array.next = NULL;
    json->value.array.element = NULL;

    return json;
}

void json_free(Json *json){
    if(!json){
        return;
    }

    switch(json->type){
        case JSON_STRING: {
            free(json->value.string);
            break;
        }

        case JSON_ARRAY: {
            Json *current = json->value.array.element;
            while(current){
                Json *next = current->value.array.next;
                json_free(current);
                current = next;
            }
            break;
        }

        case JSON_OBJECT: {
            Json *current = json->value.object.next;
            while(current){
                Json *next = current->value.object.next;
                free(current->value.object.key);
                json_free(current->value.object.value);
                free(current);
                current = next;
            }
            break;
        }
        default:
            break;
    }

    free(json);
}

int json_array_add(Json *array, Json *value){
    if(array->type != JSON_ARRAY || !array) return -1;

    if(!array->value.array.element){
        array->value.array.element = value;
    } else {
        Json *current = array->value.array.element;
        while(current->value.array.next){
            current = current->value.array.next;
        }
        current->value.array.next = value;
    }

    return 0;
}

int json_object_add(Json *object, const char *key, Json *value){
    if(object->type != JSON_OBJECT) return -1;

    Json *new_pair = json_create_object();
    if(!new_pair) return -2;

    new_pair->value.object.key = strdup(key);
    if(!new_pair->value.object.key) {
        free(new_pair);
        return -3;
    }
    new_pair->value.object.value = value;

    new_pair->value.object.next = object->value.object.next;
    object->value.object.next = new_pair;


    return 0;
}

char *json_print(Json *json){
    size_t max_size = 128;
    size_t pos = 0;

    char *buffer = calloc(max_size, sizeof(char));
    if(!buffer) return NULL;

    json_to_string(json, &buffer, &pos, &max_size);

    buffer[pos] = '\0';
    return buffer;
}
