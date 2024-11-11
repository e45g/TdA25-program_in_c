#include <stdlib.h>

#include "../db.h"
#include "../utils.h"
#include "../server.h"
#include "../json/json.h"
#include "tictactoe.h"
#include "api.h"
#include "be_utils.h"

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
        LOG("Error parsing json: \n%s", req->body);
        send_json_error(client_fd, (ResponseInfo){ERR_BADREQ, "Error parsing json"});
        return;
    }

    char id[37] = {0};
    generate_id(id);

    const char *name = 0;
    const char *difficulty = 0;
    char board[225] = {0};
    char board_array[15][15] = {0};
    int turn = 0;
    int round = 0;

    ResponseInfo result = get_params(json, &name, &difficulty, board_array, board, &turn, &round);
    if(result.status != OK_OK){
        LOG("GET PARAMS NOT OK.");
        json_free(json);
        send_json_error(client_fd, (ResponseInfo){result.status, result.message});
        return;
    }

    char date[64] = {0};
    get_current_time(date, 64, 0);

    char game_state[16] = {0};
    get_game_state(game_state, board_array, turn ? 'X' : 'O', round);



    char *sql = "INSERT INTO games (id, created_at, updated_at, name, difficulty, game_state, board) VALUES (?, ?, ?, ?, ?, ?, ?)";
    const char *params[] = {
        id,
        date,
        date,
        name,
        difficulty,
        game_state,
        board
    };

    int rc = db_execute(sql, params, 7);
    if(rc != 0){
        LOG("Failed to insert into games");
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});

        json_free(json);
        return;
    }

    if(
        json_object_add_string(json, "uuid", id) ||
        json_object_add_string(json, "createdAt", date) ||
        json_object_add_string(json, "updatedAt", date) ||
        json_object_add_string(json, "gameState", game_state)
        != 0
    ){
        LOG("Failed json_object_add_string");

        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        json_free(json);
        return;
    }

    char *json_str = json_print(json);

    send_json_response(client_fd, 201, json_str);
    free(json_str);
    json_free(json);
}

void handle_game_update(int client_fd, HttpRequest *req){
    const char *id = req->wildcards[0];
    if(!db_exists(id)){
        send_json_error(client_fd, (ResponseInfo){ERR_NOTFOUND, "Resource not found"});
        return;
    }

    Json *json = json_parse(req->body);
    if(!json){
        send_json_error(client_fd, (ResponseInfo){ERR_BADREQ, "Error parsing json"});
        return;
    }

    const char *name = 0;
    const char *difficulty = 0;
    char board[225] = {0};
    char board_array[15][15] = {0};
    int turn = 0;
    int round = 0;

    ResponseInfo result = get_params(json, &name, &difficulty, board_array, board, &turn, &round);
    if(result.status != OK_OK){
        json_free(json);
        send_json_error(client_fd, (ResponseInfo){result.status, result.message});
        return;
    }

    char date[64] = {0};
    get_current_time(date, 64, 0);

    char game_state[16] = {0};
    get_game_state(game_state, board_array, turn ? 'X' : 'O', round);



    const char *sql = "UPDATE games SET updated_at = ?, name = ?, difficulty = ?, board = ?, game_state = ? WHERE id = ?";
    const char *params[] = {
        date,
        name,
        difficulty,
        board,
        game_state,
        id
    };

    int rc = db_execute(sql, params, 6);
    if(rc != 0){
        json_free(json);
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    if(
        json_object_add_string(json, "uuid", id) ||
        json_object_add_string(json, "updatedAt", date) ||
        json_object_add_string(json, "gameState", game_state)
        != 0
    ) {
        json_free(json);
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    DBResult *r = db_query("SELECT created_at FROM games WHERE id = ?", (const char**){&id}, 1);
    if(!r){
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        json_free(json);
        return;
    }

    rc = json_object_add_string(json, "createdAt", r->rows[0][0]);
    if(rc != 0){
        json_free(json);
        free_result(r);
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    char *json_str = json_print(json);

    send_json_response(client_fd, 200, json_str);
    free(json_str);
    free_result(r);
    json_free(json);
}

void handle_get_game(int client_fd, HttpRequest *req){
    const char *id = req->wildcards[0];

    if(!db_exists(id)) {
        send_json_error(client_fd, (ResponseInfo){ERR_BADREQ, ""});
        return;
    }

    DBResult *result = db_query("SELECT created_at, updated_at, name, difficulty, game_state, board FROM games WHERE id = ?", (const char **){&id}, 1);
    if(!result){
        LOG("DB error - get_game");
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    Json *response = json_create_object();
    if(!response){
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
        free_result(result);
        return;
    }

    Json *board = load_board(result->rows[0][5]);
    if(!board){
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
        free_result(result);
        return;
    }


    if(
        json_object_add_string(response, "uuid", id) ||
        json_object_add_string(response, "createdAt", result->rows[0][0]) ||
        json_object_add_string(response, "updatedAt", result->rows[0][1]) ||
        json_object_add_string(response, "name", result->rows[0][2]) ||
        json_object_add_string(response, "difficulty", result->rows[0][3]) ||
        json_object_add_string(response, "game_state", result->rows[0][4]) ||
        json_object_add(response, "board", board)
        != 0
    ) {
        json_free(response);
        free_result(result);

        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    free_result(result);

    char *json_str = json_print(response);

    send_json_response(client_fd, 200, json_str);
    free(json_str);
    json_free(response);
}

void handle_list_games(int client_fd, HttpRequest *req __attribute__((unused))) {
    DBResult *result = db_query("SELECT id, created_at, updated_at, name, difficulty, game_state, board FROM games", NULL, 0);
    if (!result) {
        LOG("DB error - list_games");
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    Json *response = json_create_array(0);
    if (!response) {
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
        free_result(result);
        return;
    }

    for (int i = 0; i < result->num_rows; ++i) {
        Json *game_object = json_create_object();
        if (!game_object) {
            json_free(response);
            free_result(result);
            send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
            return;
        }

        Json *board = load_board(result->rows[0][6]);
        if(!board){
            send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
            free_result(result);
            return;
        }

        if (
            json_object_add_string(game_object, "uuid", result->rows[i][0]) ||
            json_object_add_string(game_object, "createdAt", result->rows[i][1]) ||
            json_object_add_string(game_object, "updatedAt", result->rows[i][2]) ||
            json_object_add_string(game_object, "name", result->rows[i][3]) ||
            json_object_add_string(game_object, "difficulty", result->rows[i][4]) ||
            json_object_add_string(game_object, "game_state", result->rows[i][5]) ||
            json_object_add(game_object, "board", board)
        ) {
            json_free(game_object);
            json_free(response);
            free_result(result);
            send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
            return;
        }

        if (json_array_add(response, game_object)) {
            json_free(game_object);
            json_free(response);
            free_result(result);
            send_json_error(client_fd, (ResponseInfo){ERR_INTERR, ":("});
            return;
        }
    }


    free_result(result);

    char *json_str = json_print(response);

    send_json_response(client_fd, 200, json_str);

    free(json_str);
    json_free(response);
}

void handle_game_deletion(int client_fd, HttpRequest *req){
    const char *id = req->wildcards[0];

    int result = db_exists(id);
    if(result != 0){
        send_json_error(client_fd, (ResponseInfo){ERR_NOTFOUND, "Resource not found"});
        return;
    }

    result = db_execute("DELETE FROM games WHERE id = ?", (const char **){&id}, 1);
    if(result != 0){
        send_json_error(client_fd, (ResponseInfo){ERR_INTERR, "DB Error"});
        return;
    }

    send_json_response(client_fd, OK_NOCONTENT, "{\"code\": 204, \"message\": \"Deleted\"}");
}
