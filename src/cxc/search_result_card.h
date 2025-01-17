#ifndef search_result_card
#define search_result_card
#include "../game.h"

typedef struct {
    
    game_t *game;

} SearchResultCardProps;

char *render_search_result_card(SearchResultCardProps *props);

#endif
