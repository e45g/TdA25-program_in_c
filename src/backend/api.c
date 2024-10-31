#include <stdio.h>

#include "api.h"
#include "be_utils.h"
#include "../db.h"
#include "../utils.h"
#include "../server.h"
#include "../lib/cJSON/cJSON.h"

void handle_api(int client_fd, HttpRequest *req __attribute__((unused))) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "organization", "Student Cyber Games");
    char *j = cJSON_Print(json);

    send_json_response(client_fd, 200, j);

    cJSON_free(j);
    cJSON_Delete(json);
}

void handle_game_creation(int client_fd, HttpRequest *req){
    cJSON *json = cJSON_Parse(req->body);
    if(!json){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Error while parsing json\"}");
        return;
    }

    char id[33];
    generate_id(id);

    const char *name = NULL;
    const char *difficulty = NULL;
    char game_state[16] = {0};
    char date[64] = {0};
    char board_str[225] = {0};
    char board_array[15][15] = {0};


    if(get_create_update_params(client_fd, json, &name, &difficulty, game_state, date, board_str, board_array) < 0){
        return;
    }

    const char *sql = "INSERT INTO games (id, created_at, updated_at, name, difficulty, game_state, board) VALUES (?, ?, ?, ?, ?, ?, ?)";
    const char *params[] = {
        id,
        date,
        date,
        name,
        difficulty,
        game_state,
        board_str
    };

    if(execute_sql_with_placeholders(sql, params, 7) != 0){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error.\"}");
        cJSON_Delete(json);
        return;
    }


    // TODO : Read from DB
    cJSON_AddStringToObject(json, "uuid", id);
    cJSON_AddStringToObject(json, "createdAt", date);
    cJSON_AddStringToObject(json, "updatedAt", date);
    cJSON_AddStringToObject(json, "gameState", game_state);

    char *json_str = cJSON_Print(json);

    send_json_response(client_fd, 201, json_str);
    cJSON_free(json_str);
    cJSON_Delete(json);

}

void handle_game_update(int client_fd, HttpRequest *req){
    cJSON *json = cJSON_Parse(req->body);
    if(!json){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Error while parsing json\"}");
        return;
    }

    const char *id = req->wildcards[0];
    const char *name = NULL;
    const char *difficulty = NULL;
    char game_state[16] = {0};
    char date[64] = {0};
    char board_str[225] = {0};
    char board_array[15][15] = {0};

    if(!exists(id)){
        send_json_response(client_fd, ERR_NOTFOUND, "{\"code\": 404, \"message\": \"Resource not found.\"}");
        cJSON_Delete(json);
        return;
    }

    if(get_create_update_params(client_fd, json, &name, &difficulty, game_state, date, board_str, board_array) < 0){
        return;
    }

    const char *sql = "UPDATE games SET updated_at = ?, name = ?, difficulty = ?, board = ?, game_state = ? WHERE id = ?";
    const char *params[] = {
        date,
        name,
        difficulty,
        board_str,
        game_state,
        id
    };

    if(execute_sql_with_placeholders(sql, params, 6) != 0){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        cJSON_Delete(json);
        return;
    }


    cJSON_AddStringToObject(json, "uuid", id);
    cJSON_AddStringToObject(json, "updatedAt", date);
    cJSON_AddStringToObject(json, "gameState", game_state);

    DBResult *result = db_query("SELECT created_at FROM games WHERE id = ?", (const char**){&id}, 1);
    if(!result){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        cJSON_Delete(json);
        return;
    }

    cJSON_AddStringToObject(json, "createdAt", result->rows[0][0]);

    char *json_str = cJSON_Print(json);

    send_json_response(client_fd, 201, json_str);
    cJSON_free(json_str);
    cJSON_Delete(json);
}

void handle_get_game(int client_fd, HttpRequest *req){
    const char *id = req->wildcards[0];
    if(!exists(id)){
        send_json_response(client_fd, ERR_NOTFOUND, "{\"code\": 404, \"message\": \"Resource not found.\"}");
        return;
    }

    DBResult *result = db_query("SELECT created_at, updated_at, name, difficulty, game_state, board FROM games WHERE id = ?", (const char **){&id}, 1);
    if(!result){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        return;
    }

    cJSON *json = cJSON_CreateObject();
    if(!json){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal Error\"}");
        return;
    }

    char ***rows = result->rows;

    // I did this only to realize i dont need it
    //
    // char board[15][15] = {0};
    // char *s = rows[0][5];
    // int j = 0;
    // while(*s){
    //     if(j / 15 >= 15) break;
    //     *(*(board+(j/15))+(j%15)) = *s++;j++; // Just flexing
    // }
    //


    cJSON *json_board = cJSON_CreateArray();
    for(int i = 0; i < 15; i++){
        cJSON *row_array = cJSON_CreateArray();
        for(int j = 0; j < 15; j++){
            char value = rows[0][5][i*15+j];
            char value_str[2] = {value, '\0'};
            cJSON *str = cJSON_CreateString(value == ' ' ? "" : value_str);

            cJSON_AddItemToArray(row_array, str);
        }
        cJSON_AddItemToArray(json_board, row_array);
    }


    cJSON_AddStringToObject(json, "uuid", id);
    cJSON_AddStringToObject(json, "createdAt", rows[0][0]);
    cJSON_AddStringToObject(json, "updatedAt", rows[0][1]);
    cJSON_AddStringToObject(json, "name", rows[0][2]);
    cJSON_AddStringToObject(json, "difficulty", rows[0][3]);
    cJSON_AddStringToObject(json, "game_state", rows[0][4]);
    cJSON_AddItemToObject(json, "board", json_board);

    char *json_str = cJSON_Print(json);

    free_result(result);
    send_json_response(client_fd, OK_OK, json_str);


    cJSON_free(json_str);
    cJSON_Delete(json);
}
