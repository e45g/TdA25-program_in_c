#ifndef API_H
#define API_H

#include "../server.h"

void handle_api(int client_fd, http_req_t *req);
void handle_game_creation(int client_fd, http_req_t *req);
void handle_game_update(int client_fd, http_req_t *req);
void handle_get_game(int client_fd, http_req_t *req);
void handle_list_games(int client_fd, http_req_t *req);
void handle_game_deletion(int client_fd, http_req_t *req);

#endif
