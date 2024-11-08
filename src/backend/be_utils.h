#ifndef BE_UTILS_H
#define BE_UTILS_H

#include "../json/json.h"

int load_board(int client_fd, Json *json, char board_str[225], char board_array[15][15], char *player, int *round);
int get_create_update_params(int client_fd, Json *json,const char **name, const char **difficulty, char *game_state, char *date, char board_str[225], char board_array[15][15]);
Json *get_game(int client_fd, const char *id);
Json *get_all_games(int client_fd);

#endif
