#ifndef API_H
#define API_H

#include "../server.h"

void handle_api(int client_fd, HttpRequest *req);
void handle_game_creation(int client_fd, HttpRequest *req);
void handle_game_update(int client_fd, HttpRequest *req);
void handle_get_game(int client_fd, HttpRequest *req);

#endif
