#ifndef BE_UTILS_H
#define BE_UTILS_H

#include "../json/json.h"
#include "../server.h"

ResponseInfo get_params(Json *json, const char **name, const char **difficulty, char board_array[15][15], char board[225], int *turn, int *round);
void send_json_error(int client_fd, ResponseInfo info);
Json *load_board(char *board_str);

#endif
