#include "json/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "db.h"
#include "game.h"
#include "server.h"
#include "backend/api.h"
#include "utils.h"

#include "cxc/game_page.h"
#include "cxc/layout.h"
#include "cxc/search_result_list.h"

extern server_t server;

int match_route(const char *route, const char *handle)
{
    const char *r = route;
    const char *h = handle;
    while (*r && *h)
    {
        if (*h == '*')
        {
            r = strchr(r, '/');
            if (!r)
            {
                return 1;
            }
            h++;
            continue;
        }
        else if (*h != *r)
        {
            return 0;
        }
        h++;
        r++;
    }

    return (*r == '\0' && (*h == '\0' || *h == '*'));
}

void get_wildcards(http_req_t *req, const route_t *r)
{
    const char *req_path = req->path;
    const char *route_path = r->path;

    int req_len = strlen(req_path);
    int route_len = strlen(route_path);

    int i = 0;
    int j = 0;

    req->wildcard_num = 0;

    while (i < req_len && j < route_len)
    {
        if (route_path[j] == '*')
        {
            j++;
            if (req->wildcard_num < 16)
            {
                int k = 0;
                while (i < req_len && req_path[i] != '/' && k < 63)
                {
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

void add_route(const char *method, const char *path, void (*callback)(int client_fd, http_req_t *req))
{
    route_t *r = malloc(sizeof(route_t));
    if (r == NULL)
    {
        LOG("Failed to allocate memory");
        return;
    }
    strncpy(r->method, method, sizeof(r->method) - 1);
    strncpy(r->path, path, sizeof(r->path) - 1);
    r->callback = callback;
    r->next = server.route;
    server.route = r;
}

void print_routes(void)
{
    for (route_t *r = server.route; r; r = r->next)
    {
        LOG("route_t - %s: %s", r->method, r->path);
    }
}

void free_routes(void)
{
    route_t *current = server.route;
    while (current)
    {
        route_t *tmp = current->next;
        free(current);
        current = tmp;
    }
}

void handle_root(int client_fd, http_req_t *req __attribute__((unused)))
{
    send_string(client_fd, "Hello TdA");
}

void handle_test(int client_fd, http_req_t *req __attribute__((unused)))
{
    char date[64];
    get_current_time(date, 64, -300);

    const char *params[] = {
        date
    };

    db_result_t *result = db_query("SELECT * FROM games WHERE created_at >= ?", params, 1);
    if (!result)
        return;
    printf("%d %d\n", result->num_cols, result->num_rows);
    for (int i = 0; i < result->num_rows; i++)
    {
        for (int j = 0; j < result->num_cols; j++)
        {
            printf("%s ", result->rows[i][j]);
        }
        printf("\n");
    }
    free_result(result);

    send_string(client_fd, "look at em");
}

void handle_log(int client_fd, http_req_t *req __attribute__((unused)))
{
    serve_file(client_fd, "../log.txt");

}

void handle_game(int client_fd, http_req_t *req __attribute__((unused)))
{

    db_result_t *dummy = 0;
    size_t game_count = 0;
    game_t *games = get_game(NULL, NULL, NULL, &game_count, &dummy);

    GamePageProps game_page_props = {
        .length=(int) game_count,
        .game = games
    };

    char *game_page_str = render_game_page(&game_page_props);

    LayoutProps layout_props = {
        .children = game_page_str,
        .page = 1
    };

    char *layout_str = render_layout(&layout_props);

    send_string(client_fd, layout_str);

    free(game_page_str);
    free(layout_str);
    free_result(dummy);
    free(games);
}

void handle_search(int client_fd, http_req_t *req __attribute__((unused))) {
    json_t *json = json_parse(req->body);

    char *name = json_object_get_string(json, "name");
    char *difficulty = json_object_get_string(json, "difficulty");
    char *date = json_object_get_string(json, "date");

    db_result_t *dummy = 0;
    size_t game_count = 0;
    game_t *games = get_game(name, difficulty, date, &game_count, &dummy);

    SearchResultListProps search_result_props = {
        .game = games,
        .length = (int) game_count,
    };
    char *list = render_search_result_list(&search_result_props);

    send_string(client_fd, list);

    free(list);

    free(games);
    free_result(dummy);
    json_free(json);
}

void load_routes(void)
{
    add_route("GET", "/", handle_game);

    // api
    add_route("GET", "/api", handle_api);
    add_route("POST", "/api/v1/games", handle_game_creation);
    add_route("PUT", "/api/v1/games/*", handle_game_update);
    add_route("DELETE", "/api/v1/games/*", handle_game_deletion);
    add_route("GET", "/api/v1/games/*", handle_get_game);
    add_route("GET", "/api/v1/games", handle_list_games);

    add_route("GET", "/test", handle_test);
    add_route("GET", "/game", handle_game);
    add_route("GET", "/game/*", handle_game);

    add_route("GET", "/log", handle_log);
    add_route("POST", "/game/search", handle_search);
}

