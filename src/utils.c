#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#include "utils.h"
#include "lib/cJSON/cJSON.h"
#include "server.h"

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

int get_port(){
    const char *port = getenv("PORT");
    return port ? strtol(port, NULL, 10) : 1444;
}

const char *get_routes_dir(){
    const char *dir = getenv("ROUTES_DIR");
    return dir ? dir : "./routes";
}

const char *get_public_dir(){
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


void generate_id(char *buffer) {
    char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789_";
    srand((unsigned int)time(NULL));

    for (int i = 0; i < 32; i++) {
        buffer[i] = charset[rand() % strlen(charset)];
    }
    buffer[32] = '\0';
}

void get_current_time(char *buffer, size_t size) {
    struct timeval tv;
    struct tm *tm_info;

    gettimeofday(&tv, NULL);
    tm_info = gmtime(&tv.tv_sec);

    strftime(buffer, size, "%Y-%m-%dT%H:%M:%S", tm_info);
    snprintf(buffer + 19, size - 19, ".%03ldZ", tv.tv_usec / 1000);
}

int load_board(int client_fd, cJSON *json, char board_str[225], char board_array[15][15], char *player, int *round){
    cJSON *json_board = cJSON_GetObjectItem(json, "board");
    if(!json_board || !cJSON_IsArray(json_board)){
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Board is missing or not an array.\"}");
        cJSON_Delete(json);
        return -1;
    }

    int x=0, o=0;
    for (int i = 0; i < 15; i++) {
        cJSON *row = cJSON_GetArrayItem(json_board, i);
        if (row != NULL && cJSON_IsArray(row)) {
            for (int j = 0; j < 15; j++) {
                cJSON *cell = cJSON_GetArrayItem(row, j);
                if (cell != NULL && cJSON_IsString(cell)) {
                    char *s = cell->valuestring;
                    if(strcmp(s, "X") == 0){
                        x++;
                        strcat(board_str, s);
                        board_array[i][j] = *s;
                    }
                    else if(strcmp(s, "O") == 0){
                        o++;
                        strcat(board_str, s);
                        board_array[i][j] = *s;
                    }
                    else if (strcmp(s, "") == 0){
                        strcat(board_str, " ");
                        board_array[i][j] = ' ';
                    }
                    else{
                        send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Place only X or O.\"}");
                        cJSON_Delete(json);
                        return -1;
                    }
                } else {
                    send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
                    cJSON_Delete(json);
                    return -1;
                }
            }
        }else{
            send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
            cJSON_Delete(json);
            return -1;
        }
    }

    if(o > x || x-1 > o){
        send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Place equal amount of symbols || x+1 == o.\"}");
        cJSON_Delete(json);
        return -1;
    }

    *player = x-o ? 'O' : 'X';
    *round = x;

    return 0;
}
