#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "lib/cJSON/cJSON.h"
#include "db.h"
#include "utils.h"

extern Server server;

int match_route(const char *route, const char *handle) {
    const char *r = route;
    const char *h = handle;
    while (*r && *h) {
        if (*h == '*') {
            r = strchr(r, '/');
            if (!r) {
                return 1;
            }
            h++;
            continue;
        }
        else if (*h != *r) {
            return 0;
        }
        h++;
        r++;
    }

    return (*r == '\0' && (*h == '\0' || *h == '*'));
}

void add_route(const char *method, const char *path, void (*callback)(int client_fd, HttpRequest *req)) {
    Route *r = malloc(sizeof(Route));
    if (r == NULL) {
        perror("Failed to allocate memory");
        return;
    }
    strncpy(r->method, method, sizeof(r->method) - 1);
    strncpy(r->path, path, sizeof(r->path) - 1);
    r->callback = callback;
    r->next = server.route;
    server.route = r;
}

void print_routes() {
    for (Route *r = server.route; r; r = r->next)
    {
        printf("Route - %s: %s\n", r->method, r->path);
    }
}

void free_routes() {
    Route *current = server.route;
    while (current)
    {
        Route *tmp = current->next;
        free(current);
        current = tmp;
    }
}

void handle_root(int client_fd, HttpRequest *req __attribute__((unused))) {
    send_string(client_fd, "Hello TdA");
}

void handle_api(int client_fd, HttpRequest *req __attribute__((unused))) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "organization", "Student Cyber Games");
    char *j = cJSON_Print(json);

    send_json_response(client_fd, j);

    cJSON_free(j);
    cJSON_Delete(json);
}

void try_to_handle_game_creation(int client_fd, HttpRequest *req){
    cJSON *json = cJSON_Parse(req->body);
    if(!json){
        send_json_response(client_fd, "{\"code\": 400, \"message\": \"error parsing json\"}");
        return;
    }
    char *json_str = cJSON_Print(json);

    char board[225];
    cJSON *json_board = cJSON_GetObjectItem(json, "board");
    if(!json_board || !cJSON_IsArray(json_board)){
        send_json_response(client_fd, "{\"code\": 400, \"message\": \"Bad request: wrong correct board.\"}");
        cJSON_Delete(json);
        return;
    }

    for (int i = 0; i < 15; i++) {
        cJSON *row = cJSON_GetArrayItem(json_board, i);
        if (row != NULL && cJSON_IsArray(row)) {
            for (int j = 0; j < 15; j++) {
                cJSON *cell = cJSON_GetArrayItem(row, j);
                if (cell != NULL && cJSON_IsString(cell)) {
                    char *s = cell->valuestring;
                    if(strcmp(s, "X") == 0){
                        strcat(board, s);
                    }
                    if(strcmp(s, "O") == 0){
                        strcat(board, s);
                    }
                    else{
                        strcat(board, " ");
                    }
                } else {
                    send_json_response(client_fd, "{\"code\": 400, \"message\": \"Bad request: wrong amount of cells in the board thing.\"}");
                    cJSON_Delete(json);
                    return;
                }
            }
        }else{
            send_json_response(client_fd, "{\"code\": 400, \"message\": \"Bad request: fix your board rows.\"}");
            cJSON_Delete(json);
            return;
        }
    }

    char id[33] = {0};
    generate_id(id);

    char date[64] = {0};
    get_current_time(date, sizeof(date));

    const char *name = cjson_get_string(json, "name");
    if(!name){
        send_json_response(client_fd, "{\"code\": 400, \"message\": \"Bad request: missing name\"}");
        cJSON_Delete(json);
        return;
    }
    const char *difficulty = cjson_get_string(json, "difficulty");
    if(!difficulty){
        send_json_response(client_fd, "{\"code\": 400, \"message\": \"Bad request: missing difficulty.\"}");
        cJSON_Delete(json);
        return;
    }

    const char *sql = "INSERT INTO games (id, created_at, updated_at, name, difficulty, game_state, board) VALUES (?, ?, ?, ?, ?, ?, ?)";
    const char *params[] = {
        id,
        date,
        date,
        name,
        difficulty,
        "endgame",
        board
    };

    execute_sql_with_placeholders(sql, params, 7);

    send_json_response(client_fd, json_str);

    cJSON_free(json_str);
    cJSON_Delete(json);
}


void load_routes() {
    add_route("GET", "/", handle_root);
    add_route("GET", "/api", handle_api);
    add_route("POST", "/api/v1/games", try_to_handle_game_creation);
}
