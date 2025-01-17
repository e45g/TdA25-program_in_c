#ifndef search_result_list
#define search_result_list
#include "../game.h"

typedef struct {
    
    int length;
    game_t *game;

} SearchResultListProps;

char *render_search_result_list(SearchResultListProps *props);

#endif
