#include <string.h>

#include "../lib/cJSON/cJSON.h"
#include "../server.h"
#include "tictactoe.h"
#include "../utils.h"

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
    get_current_time(date, 64);


    return 0;
}
