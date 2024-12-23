#include <stdio.h>
#include <string.h>

#include "../json/json.h"
#include "../server.h"
#include "../utils.h"

void send_json_error(int client_fd, res_info_t info){
    char response[256] = {0};
    switch(info.status){
        case ERR_UNPROC:
            snprintf(response, sizeof(response), "{\"code\": %d, \"message\": \"Semantic error: %s\"}", info.status, info.message);
            break;
        case ERR_BADREQ:
            snprintf(response, sizeof(response), "{\"code\": %d, \"message\": \"Bad request: %s\"}", info.status, info.message);
            break;
        case ERR_NOTFOUND:
            snprintf(response, sizeof(response), "{\"code\": %d, \"message\": \"Resource not found\"}", info.status);
            break;
        default:
            snprintf(response, sizeof(response), "{\"code\": %d, \"message\": \"Unknown: %s\"}", info.status, info.message);
    }

    info.message = response;
    send_json_response(client_fd, info.status, info.message);
}

res_info_t get_params(json_t *json, const char **name, const char **difficulty, char board_array[15][15], char board[225], int *turn, int *round){
    if(!json) return (res_info_t){ERR_BADREQ, "Bad json"};

    const char *json_name = json_object_get_string(json, "name");
    if(!json_name)
    {
        LOG("Missing name");
        return (res_info_t){ERR_BADREQ, "Missing 'name'"};
    }
    *name = json_name;

    const char *json_difficulty = json_object_get_string(json, "difficulty");
    if(!json_difficulty) {
        LOG("Missing difficulty");
        return (res_info_t){ERR_BADREQ, "Missing 'difficulty'"};
    }
    *difficulty = json_difficulty;

    json_t *json_board = json_object_get_array(json, "board");
    if(!json_board) {
        LOG("Board missing or not an array");
        return (res_info_t){ERR_BADREQ, "Either 'board' is missing or is not an array"};
    }

    int x = 0;
    int o = 0;

    for(int i = 0; i < 15; i++){
        json_t *row = json_array_get(json_board, i);
        if(!row || !json_is_array(row)) return (res_info_t){ERR_UNPROC, "Board is missing rows"};

        for(int j = 0; j < 15; j++){
            json_t *cell = json_array_get(row, j);
            if(!cell || !json_is_string(cell)) return (res_info_t){ERR_UNPROC, "Board is missing cells"};

            char *s = cell->value.string;
            if(*s == '\0'){
                strcat(board, " ");
                board_array[i][j] = ' ';
            }
            else if(*s == 'X') {
                x++;
                strcat(board, "X");
                board_array[i][j] = 'X';
            }
            else if(*s == 'O') {
                o++;
                strcat(board, "O");
                board_array[i][j] = 'O';
            }
            else return (res_info_t){ERR_UNPROC, "Place only X, O and "};
        }
    }
    if(o > x || o+1 < x) return (res_info_t){ERR_UNPROC, "Unfair"};
    *round = x;
    *turn = x-o ? 0 : 1; // 1 for X; 0 for O

    return (res_info_t){OK_OK, "Success"};
}

json_t *load_board(char *board_str){
    json_t *board = json_create_array(0);
    if(!board){
            LOG("board fialed to create");
            return NULL;
    }

    for(int i = 0; i < 15; i++){
        json_t *row = json_create_array(0);
        if(!row){
            LOG("row fialed to create");
            return NULL;
        }

        for(int j = 0; j < 15; j++){
            char value = board_str[i*15+j];
            char value_str[2] = {value, '\0'};
            json_t *str = json_create_string(value == ' ' ? "" : value_str);

            json_array_add(row, str);
            // no error checking, it will definitely work
        }
        json_array_add(board, row);
    }
    return board;
}
