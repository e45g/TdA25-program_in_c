#include "game_page.h"

#include <stdlib.h>
#include "../cx.h"


#include <stdlib.h>
#include "search_result_list.h"


char *render_game_page(GamePageProps *props)
{
    char *output = calloc(263063+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<main class=\"flex h-screen\">    <div class=\"grow flex items-center justify-center\">        <div id=\"board\" class=\"w-2xl\">        </div>    </div>    <div class=\"w-md bg-white border-l-2 border-border px-4\">        <h2>Vyhledávání</h2>        <div class=\"flex gap-4 mb-4\">            <button class=\"primary icon\">                <div class=\"icon-filter\"></div>            </button>            <input id=\"game-search-input\" class=\"\" />            <button class=\"primary icon\">                <div class=\"icon-search\"></div>            </button>        </div>        ");
	
            SearchResultListProps search_result_props = {
                .x = 10
            };
            char *list = render_search_result_list(&search_result_props);
            fast_strcat(output, list);
            free(list);
        
	fast_strcat(output, "    </div>    <script src=\"/dist/modules/game/main.js\"></script></main>");


    return output;
}
