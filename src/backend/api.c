#include "api.h"

#include "tictactoe.h"
#include "../db.h"
#include "../utils.h"
#include "../server.h"
#include "../lib/cJSON/cJSON.h"

int get_create_params(int client_fd, cJSON *json,const char **name, const char **difficulty, char *game_state, char *date, char board_str[225], char board_array[15][15]){
    char player;
    int round = 0;
    int r = load_board(client_fd, json, board_str, board_array, &player, &round);
    if(r < 0) return -1;

    const char *p_name = cjson_get_string(json, "name");
    if(!p_name){
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Bad request: missing name\"}");
        cJSON_Delete(json);
        return -1;
    }
    *name = p_name;

    const char *p_difficulty = cjson_get_string(json, "difficulty");
    if(!p_difficulty){
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Bad request: missing difficulty\"}");
        cJSON_Delete(json);
        return -1;
    }
    *difficulty = p_difficulty;


    get_game_state(game_state, board_array, player, round);
    get_current_time(date, 64);


    return 0;
}

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

    char id[33];
    generate_id(id);

    const char *name = NULL;
    const char *difficulty = NULL;
    char game_state[16] = {0};
    char date[64] = {0};
    char board_str[225] = {0};
    char board_array[15][15] = {0};


    if(get_create_params(client_fd, json, &name, &difficulty, game_state, date, board_str, board_array) < 0){
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
        send_json_response(client_fd, 400, "{\"code\": 500, \"message\": \"DB error.\"}");
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
        send_json_response(client_fd, 400, "{\"code\": 400, \"message\": \"Error while parsing json\"}");
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
        send_json_response(client_fd, 404, "{\"code\": 404, \"message\": \"Resource not found.\"}");
        cJSON_Delete(json);
        return;
    }

    if(get_create_params(client_fd, json, &name, &difficulty, game_state, date, board_str, board_array) < 0){
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
        send_json_response(client_fd, 500, "{\"code\": 500, \"message\": \"DB error\"}");
        cJSON_Delete(json);
        return;
    }


    // TODO : Read from DB, missing createdAt
    cJSON_AddStringToObject(json, "uuid", id);
    cJSON_AddStringToObject(json, "updatedAt", date);
    cJSON_AddStringToObject(json, "gameState", game_state);

    char *json_str = cJSON_Print(json);

    send_json_response(client_fd, 201, json_str);
    cJSON_free(json_str);
    cJSON_Delete(json);
}


