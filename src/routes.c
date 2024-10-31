#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "backend/api.h"

extern Server server;

int match_route(const char *route, const char *handle) {
    const char *r = route;
    const char *h = handle;
    while (*r && *h) {
        if (*h == '*') {
            r = strchr(r, '/');
            if (!r) {
                return 1;
            }
            h++;
            continue;
        }
        else if (*h != *r) {
            return 0;
        }
        h++;
        r++;
    }

    return (*r == '\0' && (*h == '\0' || *h == '*'));
}

void get_wildcards(HttpRequest *req, const Route *r){
    const char *req_path = req->path;
    const char *route_path = r->path;

    int req_len = strlen(req_path);
    int route_len = strlen(route_path);

    int i = 0;
    int j = 0;

    req->wildcard_num = 0;

    while(i < req_len && j < route_len){
        if(route_path[j] == '*'){
            j++;
            if(req->wildcard_num < 16){
                int k = 0;
                while(i < req_len && req_path[i] != '/' && k < 63){
                    req->wildcards[req->wildcard_num][k++] = req_path[i++];
                }
                req->wildcards[req->wildcard_num][k] = '\0';
                req->wildcard_num++;
            }
        }
        i++;
        j++;
    }
}

void add_route(const char *method, const char *path, void (*callback)(int client_fd, HttpRequest *req)) {
    Route *r = malloc(sizeof(Route));
    if (r == NULL) {
        perror("Failed to allocate memory");
        return;
    }
    strncpy(r->method, method, sizeof(r->method) - 1);
    strncpy(r->path, path, sizeof(r->path) - 1);
    r->callback = callback;
    r->next = server.route;
    server.route = r;
}

void print_routes() {
    for (Route *r = server.route; r; r = r->next)
    {
        printf("Route - %s: %s\n", r->method, r->path);
    }
}

void free_routes() {
    Route *current = server.route;
    while (current)
    {
        Route *tmp = current->next;
        free(current);
        current = tmp;
    }
}

void handle_root(int client_fd, HttpRequest *req __attribute__((unused))) {
    send_string(client_fd, "Hello TdA");
}

void load_routes() {
    add_route("GET", "/", handle_root);
    add_route("GET", "/api", handle_api);

    add_route("POST", "/api/v1/games", handle_game_creation);
    add_route("PUT", "/api/v1/games/*", handle_game_update);
    add_route("GET", "/api/v1/games/*", handle_get_game);
}
