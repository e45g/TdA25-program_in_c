#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "lib/cJSON/cJSON.h"
#include "utils.h"

#include "cxc/layout.h"

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

void handle_api(int client_fd, HttpRequest *req __attribute__((unused))) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "organization", "Student Cyber Games");
    char *j = cJSON_Print(json);

    send_json_response(client_fd, j);

    cJSON_free(j);
    cJSON_Delete(json);

}


void load_routes() {
    add_route("GET", "/", handle_root);
    add_route("GET", "/api", handle_api);
}
