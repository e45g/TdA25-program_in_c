#include "game.h"
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


game_t *get_game(char *name, char *difficulty, char *updated_at, size_t *game_count, db_result_t **p) {
    char query[256] = "SELECT id, created_at, updated_at, name, difficulty, game_state FROM games";

    char *params[3] = {};
    int params_count = 0;

    if(name != NULL || difficulty != NULL || updated_at != NULL) {
        strcat(query, " WHERE ");
    }

    if(name != NULL) {
        strcat(query, "name LIKE ?");
        char *like_name = malloc(strlen(name) + 4);
        sprintf(like_name, "%%%s%%", name);
        params[params_count++] = like_name;
    }
    if(difficulty != NULL) {
        if(params_count >= 1) strcat(query, " AND ");
        strcat(query, "difficulty = ?");
        params[params_count++] = difficulty;
    }
    if(updated_at != NULL) {
        if(params_count >= 1) strcat(query, " AND ");
        strcat(query, "updated_at >= ?");
        params[params_count++] = updated_at;
    }

    printf("%s; %d\n", query, params_count);
    for (int i = 0; i < params_count; i++) {
        printf("%s \n", params[i]);
    }

    db_result_t *result = db_query(query, (const char **)params, params_count);
    if(result->num_rows == 0) return NULL;

    *p = result;
    *game_count = result->num_rows;

    game_t *games = calloc(result->num_rows, sizeof(game_t));
    size_t current_game = 0;

    if (name != NULL) {
        free(params[0]);
    }

    for (int row = 0; row < result->num_rows; row++) {
        games[current_game].id = result->rows[row][0];
        games[current_game].created_at = result->rows[row][1];
        games[current_game].updated_at = result->rows[row][2];
        games[current_game].name = result->rows[row][3];
        games[current_game].difficulty = result->rows[row][4];
        games[current_game].game_state = result->rows[row][5];
        current_game++;
    }

    return games;
}
