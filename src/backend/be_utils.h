#ifndef BE_UTILS_H
#define BE_UTILS_H

#include "../json/json.h"
#include "../server.h"

res_info_t get_params(json_t *json, const char **name, const char **difficulty, char board_array[15][15], char board[225], int *turn, int *round);
void send_json_error(int client_fd, res_info_t info);
json_t *load_board(char *board_str);

#endif
