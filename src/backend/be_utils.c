#include <string.h>

#include "../lib/cJSON/cJSON.h"
#include "../server.h"
#include "tictactoe.h"
#include "../utils.h"
#include "../db.h"

int load_board(int client_fd, cJSON *json, char board_str[225], char board_array[15][15], char *player, int *round){
    cJSON *json_board = cJSON_GetObjectItem(json, "board");
    if(!json_board || !cJSON_IsArray(json_board)){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Board is missing or not an array.\"}");
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
                        send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Place only X or O.\"}");
                        cJSON_Delete(json);
                        return -1;
                    }
                } else {
                    send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
                    cJSON_Delete(json);
                    return -1;
                }
            }
        }else{
            send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Board is the wrong size. (Expected: 15x15)\"}");
            cJSON_Delete(json);
            return -1;
        }
    }

    if(o > x || x-1 > o){
        send_json_response(client_fd, ERR_UNPROC, "{\"code\": 422, \"message\": \"Semantic error: Place equal amount of symbols || x+1 == o.\"}");
        cJSON_Delete(json);
        return -1;
    }

    *player = x-o ? 'O' : 'X';
    *round = x;

    return 0;
}

int get_create_update_params(int client_fd, cJSON *json,const char **name, const char **difficulty, char *game_state, char *date, char board_str[225], char board_array[15][15]){
    char player;
    int round = 0;
    int r = load_board(client_fd, json, board_str, board_array, &player, &round);
    if(r < 0) return -1;

    const char *p_name = cjson_get_string(json, "name");
    if(!p_name){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Bad request: missing name\"}");
        cJSON_Delete(json);
        return -1;
    }
    *name = p_name;

    const char *p_difficulty = cjson_get_string(json, "difficulty");
    if(!p_difficulty){
        send_json_response(client_fd, ERR_BADREQ, "{\"code\": 400, \"message\": \"Bad request: missing difficulty\"}");
        cJSON_Delete(json);
        return -1;
    }
    *difficulty = p_difficulty;


    get_game_state(game_state, board_array, player, round);
    get_current_time(date, 64, 0);


    return 0;
}


cJSON *get_game(int client_fd, const char *id){
    DBResult *result = db_query("SELECT created_at, updated_at, name, difficulty, game_state, board FROM games WHERE id = ?", (const char **){&id}, 1);
    if(!result){
        send_json_response(client_fd, ERR_INTERR, "{\"code\": 500, \"message\": \"DB error\"}");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
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

    free_result(result);
    return json;
}

