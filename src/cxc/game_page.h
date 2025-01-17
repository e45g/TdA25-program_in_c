#ifndef game_page
#define game_page
#include "../game.h"

typedef struct {
    
    int x;
    int length;
    game_t *game;

} GamePageProps;

char *render_game_page(GamePageProps *props);

#endif
