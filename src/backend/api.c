#include <string.h>

#include "tictactoe.h"
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
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Error while parsing json\"}");
        return;
    }

    char board[225] = {0};
    char board_array[15][15] = {0};
    cJSON *json_board = cJSON_GetObjectItem(json, "board");
    if(!json_board || !cJSON_IsArray(json_board)){
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Board is missing or not an array.\"}");
        cJSON_Delete(json);
        return;
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
                        strcat(board, s);
                        board_array[i][j] = *s;
                    }
                    else if(strcmp(s, "O") == 0){
                        o++;
                        strcat(board, s);
                        board_array[i][j] = *s;
                    }
                    else if (strcmp(s, "") == 0){
                        strcat(board, " ");
                        board_array[i][j] = ' ';
                    }
                    else{
                        send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Place only X or O.\"}");
                        cJSON_Delete(json);
                        return;
                    }
                } else {
                    send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
                    cJSON_Delete(json);
                    return;
                }
            }
        }else{
            send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
            cJSON_Delete(json);
            return;
        }
    }

    if(o > x || x-1 > o){
        send_json_response(client_fd, 422, "{\"code\": 422, \"message\": \"Semantic error: Place equal amount of symbols || x+1 == o.\"}");
        cJSON_Delete(json);
        return;
    }


    char id[33] = {0};
    generate_id(id);

    char date[64] = {0};
    get_current_time(date, sizeof(date));

    const char *name = cjson_get_string(json, "name");
    if(!name){
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Bad request: missing name\"}");
        cJSON_Delete(json);
        return;
    }
    const char *difficulty = cjson_get_string(json, "difficulty");
    if(!difficulty){
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Bad request: missing difficulty\"}");
        cJSON_Delete(json);
        return;
    }

    char game_state[16] = {0};
    get_game_state(game_state, board_array, x, o);

    const char *sql = "INSERT INTO games (id, created_at, updated_at, name, difficulty, game_state, board) VALUES (?, ?, ?, ?, ?, ?, ?)";
    const char *params[] = {
        id,
        date,
        date,
        name,
        difficulty,
        game_state,
        board
    };

    execute_sql_with_placeholders(sql, params, 7);


    cJSON_AddStringToObject(json, "uuid", id);
    cJSON_AddStringToObject(json, "createdAt", date);
    cJSON_AddStringToObject(json, "updatedAt", date);
    cJSON_AddStringToObject(json, "gameState", game_state);

    char *json_str = cJSON_Print(json);

    send_json_response(client_fd, 201, json_str);

    cJSON_free(json_str);
    cJSON_Delete(json);
}

