#include <stdio.h>
#include <stdlib.h>

#include "api.h"
#include "be_utils.h"
#include "../db.h"
#include "../utils.h"
#include "../server.h"
#include "../json/json.h"

void handle_api(int client_fd, HttpRequest *req __attribute__((unused))) {
    Json *json = json_create_object();
    json_object_add_string(json, "organization", "Student Cyber Games");
    char *j = json_print(json);

    send_json_response(client_fd, 200, j);

    free(j);
    json_free(json);
}

void handle_game_creation(int client_fd, HttpRequest *req){
    Json *json = json_parse(req->body);
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
        json_free(json);
        return;
    }


    json_object_add_string(json, "uuid", id);
    json_object_add_string(json, "createdAt", date);
    json_object_add_string(json, "updatedAt", date);
    json_object_add_string(json, "gameState", game_state);

    char *json_str = json_print(json);

    send_json_response(client_fd, 201, json_str);
    free(json_str);
    json_free(json);

}

void handle_game_update(int client_fd, HttpRequest *req){
    Json *json = json_parse(req->body);
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
        json_free(json);
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
        json_free(json);
        return;
    }


    json_object_add_string(json, "uuid", id);
    json_object_add_string(json, "updatedAt", date);
    json_object_add_string(json, "gameState", game_state);

    DBResult *result = db_query("SELECT created_at FROM games WHERE id = ?", (const char**){&id}, 1);
    if(!result){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        json_free(json);
        return;
    }

    json_object_add_string(json, "createdAt", result->rows[0][0]);

    char *json_str = json_print(json);

    free_result(result);
    send_json_response(client_fd, 201, json_str);
    free(json_str);
    json_free(json);
}

void handle_get_game(int client_fd, HttpRequest *req){
    const char *id = req->wildcards[0];
    if(!exists(id)){
        send_json_response(client_fd, ERR_NOTFOUND, "{\"code\": 404, \"message\": \"Resource not found.\"}");
        return;
    }
    Json *json = get_game(client_fd, id);
    char *json_str = json_print(json);

    send_json_response(client_fd, OK_OK, json_str);

    free(json_str);
    json_free(json);
}

void handle_list_games(int client_fd, HttpRequest *req __attribute__((unused))){
    DBResult *result = db_query("SELECT id FROM games;", NULL, 0);
    if(!result){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal server error\"}");
        return;
    }


    Json *json_array = get_all_games(client_fd);

    char *json_str = json_print(json_array);
    send_json_response(client_fd, OK_OK, json_str);
    json_free(json_array);
    free(json_str);
    free_result(result);
}

void handle_game_deletion(int client_fd, HttpRequest *req){
    const char *id = req->wildcards[0];

    if(!exists(id)){
        send_json_response(client_fd, ERR_NOTFOUND, "{\"code\": 404, \"message\": \"Resource not found.\"}");
        return;
    }

    if(execute_sql_with_placeholders("DELETE FROM games WHERE id = ?", (const char **){&id}, 1) != 0){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        return;
    }

    send_json_response(client_fd, OK_NOCONTENT, "{\"code\": 204, \"message\": \"Deleted\"}");
}

