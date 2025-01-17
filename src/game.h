#pragma once

#include "db.h"
#include <stddef.h>

typedef struct {
    char *id;
    char *created_at;
    char *updated_at;
    char *name;
    char *difficulty;
    char *game_state;
} game_t;

game_t *get_game(char *name, char *difficulty, char *updated_at, size_t *game_count, db_result_t **p);
