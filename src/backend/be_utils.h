#ifndef BE_UTILS_H
#define BE_UTILS_H

#include "../lib/cJSON/cJSON.h"

int load_board(int client_fd, cJSON *json, char board_str[225], char board_array[15][15], char *player, int *round);
int get_create_update_params(int client_fd, cJSON *json,const char **name, const char **difficulty, char *game_state, char *date, char board_str[225], char board_array[15][15]);

#endif
