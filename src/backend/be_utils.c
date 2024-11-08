#include <stdio.h>
#include <string.h>

#include "../json/json.h"
#include "../server.h"
#include "tictactoe.h"
#include "../utils.h"
#include "../db.h"

int load_board(int client_fd, Json *json, char board_str[225], char board_array[15][15], char *player, int *round){
    Json *json_board = json_object_get_array(json, "board");
    if(!json_board || !json_is_array(json_board)){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Board is missing or not an array.\"}");
        json_free(json);
        return -1;
    }

    int x=0, o=0;
    for (int i = 0; i < 15; i++) {
        Json *row = json_array_get(json_board, i);
        if (row != NULL && json_is_array(row)) {
            for (int j = 0; j < 15; j++) {
                Json *cell = json_array_get(row, j);
                if (cell != NULL && json_is_string(cell)) {
                    char *s = cell->value.string;
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
                        send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Place only X or O.\"}");
                        json_free(json);
                        return -1;
                    }
                } else {
                    send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
                    json_free(json);
                    return -1;
                }
            }
        }else{
            send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
            json_free(json);
            return -1;
        }
    }

    if(o > x || x-1 > o){
        send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Place equal amount of symbols || x+1 == o.\"}");
        json_free(json);
        return -1;
    }

    *player = x-o ? 'O' : 'X';
    *round = x;

    return 0;
}

int get_create_update_params(int client_fd, Json *json,const char **name, const char **difficulty, char *game_state, char *date, char board_str[225], char board_array[15][15]){
    char player;
    int round = 0;
    int r = load_board(client_fd, json, board_str, board_array, &player, &round);
    if(r < 0) return -1;

    const char *p_name = json_object_get_string(json, "name");
    if(!p_name){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Bad request: missing name\"}");
        json_free(json);
        return -1;
    }
    *name = p_name;

    const char *p_difficulty = json_object_get_string(json, "difficulty");
    if(!p_difficulty){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Bad request: missing difficulty\"}");
        json_free(json);
        return -1;
    }
    *difficulty = p_difficulty;


    get_game_state(game_state, board_array, player, round);
    get_current_time(date, 64, 0);


    return 0;
}


Json *get_game(int client_fd, const char *id){
    DBResult *result = db_query("SELECT created_at, updated_at, name, difficulty, game_state, board FROM games WHERE id = ?", (const char **){&id}, 1);
    if(!result){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        return NULL;
    }

    Json *json = json_create_object();
    if(!json){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal Error\"}");
        return NULL;
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


    Json *json_board = json_create_array(0);
    for(int i = 0; i < 15; i++){
        Json *row_array = json_create_array(0);
        for(int j = 0; j < 15; j++){
            char value = rows[0][5][i*15+j];
            char value_str[2] = {value, '\0'};
            Json *str = json_create_string(value == ' ' ? "" : value_str);

            json_array_add(row_array, str);
        }
        json_array_add(json_board, row_array);
    }


    json_object_add_string(json, "uuid", id);
    json_object_add_string(json, "createdAt", rows[0][0]);
    json_object_add_string(json, "updatedAt", rows[0][1]);
    json_object_add_string(json, "name", rows[0][2]);
    json_object_add_string(json, "difficulty", rows[0][3]);
    json_object_add_string(json, "game_state", rows[0][4]);
    json_object_add(json, "board", json_board);

    free_result(result);
    return json;
}



Json *get_all_games(int client_fd) {
    // Fetch all relevant game data at once
    DBResult *result = db_query("SELECT id, created_at, updated_at, name, difficulty, game_state, board FROM games", NULL, 0);
    if (!result) {
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        return NULL;
    }

    // Create a JSON array to store all the games
    Json *json_all_games = json_create_array(0);
    if (!json_all_games) {
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal Error\"}");
        free_result(result);
        return NULL;
    }

    char ***rows = result->rows;
    int num_rows = result->num_rows;

    // Process each game row
    for (int i = 0; i < num_rows; i++) {
        Json *json_game = json_create_object();
        if (!json_game) {
            send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal Error\"}");
            free_result(result);
            return NULL;
        }

        // Process the game board
        Json *json_board = json_create_array(0);
        if (!json_board) {
            send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal Error\"}");
            free_result(result);
            return NULL;
        }

        // Process each row of the board
        const char *board = rows[i][6]; // The board string is in column 6
        for (int row = 0; row < 15; row++) {
            Json *row_array = json_create_array(0);
            if (!row_array) {
                send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"Internal Error\"}");
                free_result(result);
                return NULL;
            }

            for (int col = 0; col < 15; col++) {
                char value = board[row * 15 + col];
                Json *str = json_create_string(value == ' ' ? "" : (char[]){value, '\0'});
                json_array_add(row_array, str);
            }
            json_array_add(json_board, row_array);
        }

        // Add the game data to the JSON object
        json_object_add_string(json_game, "uuid", rows[i][0]);
        json_object_add_string(json_game, "createdAt", rows[i][1]);
        json_object_add_string(json_game, "updatedAt", rows[i][2]);
        json_object_add_string(json_game, "name", rows[i][3]);
        json_object_add_string(json_game, "difficulty", rows[i][4]);
        json_object_add_string(json_game, "game_state", rows[i][5]);
        json_object_add(json_game, "board", json_board);

        // Add the game to the overall JSON array
        json_array_add(json_all_games, json_game);
    }

    free_result(result);
    return json_all_games;
}

