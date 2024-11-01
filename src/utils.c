#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "utils.h"
#include "lib/cJSON/cJSON.h"

void logg(long line, const char *file, const char *func, const char *format, ...){
    printf("LOG: [%s:%ld %s] ", file, line, func);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

int load_env(const char *path){
    FILE *f = fopen(path, "r");
    if(!f){
        LOG("Invalid .env file");
        return -1;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), f)) {
        *(strchr(line, '\n')) = '\0';

        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if(key && value){
            setenv(key, value, 1);
        }

    }
    return 1;
}

int get_port(void){
    const char *port = getenv("PORT");
    return port ? strtol(port, NULL, 10) : 1444;
}

const char *get_routes_dir(void){
    const char *dir = getenv("ROUTES_DIR");
    return dir ? dir : "./routes";
}

const char *get_public_dir(void){
    const char *dir = getenv("PUBLIC_DIR");
    return dir ? dir : "./public";
}

const char *cjson_get_string(cJSON *json, char *key){
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);

    if(cJSON_IsString(value) && value->valuestring != NULL){
        return value->valuestring;
    }

    return NULL;
}

int cjson_get_number(cJSON *json, char *key){
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);

    if(cJSON_IsNumber(value)){
        return cJSON_GetNumberValue(value);
    }

    return 0;
}

/*
*   Pseudo random number generator by Terry A. Davis
*/
unsigned int get_num() {
    unsigned int i;
    __asm__ __volatile__ (
        "RDTSC\n"
        "MOV %%EAX, %%EAX\n"
        "SHL $32, %%RDX\n"
        "ADD %%RDX, %%RAX\n"
        : "=a" (i)
        :
        : "%rdx"
    );
    return i;
}

void generate_id(char *buffer) {
    char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    int charset_len = strlen(charset);

    for (int i = 0; i < 32; i++) {
        buffer[i] = charset[get_num() % charset_len];
    }
    buffer[32] = '\0';
}

void get_current_time(char *buffer, size_t size, long offset) {
    struct timeval tv;
    struct tm tm_info;

    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tm_info);
    tm_info.tm_sec += offset;

    mktime(&tm_info);

    if (size > 0) {
        strftime(buffer, size, "%Y-%m-%dT%H:%M:%S", &tm_info);
        snprintf(buffer + 19, size - 19, ".%03ldZ", tv.tv_usec / 1000);
    }
}

