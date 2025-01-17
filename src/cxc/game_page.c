#include "game_page.h"

#include <stdlib.h>
#include "../cx.h"


#include <stdlib.h>
#include "search_result_list.h"


char *render_game_page(GamePageProps *props)
{
    char *output = calloc(264700+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<main class=\"flex h-full overflow-x-hidden\">    <div class=\"max-lg:shrink xl:grow flex items-center justify-center\">        <div class=\"lg:max-w-5/6 max-w-full w-[90vh] aspect-square lg:p-8 xl:p-12 box-border lg:shadow-2xl lg:rounded-2xl lg:border-2 lg:border-border\">            <div id=\"board\" class=\"lg:rounded-lg overflow-hidden\">            </div>        </div>    </div>    <div class=\"shrink-0 max-lg:fixed inset-y-0 right-0\" id=\"game-search-menu-modal\" data-state=\"closed\">        <h2>Vyhledávání</h2>        <div class=\"flex gap-4 mb-2\">            <button class=\"primary icon\" id=\"game-search-trigger\" data-state=\"closed\">                <div class=\"icon-filter\"></div>                <div class=\"icon-chevron-down\"></div>            </button>            <input id=\"game-search-input\" class=\"\" onkeyup=\"onGameSearchFormChange()\" />            <button class=\"primary icon\" onclick=\"onGameSearchFormChange()\">                <div class=\"icon-search\"></div>            </button>        </div>        <div id=\"game-search-filters\" data-state=\"closed\">                        <span class=\"text-slate-700 text-sm !mb-1\">                Obtížnost:            </span>            <div>                <select id=\"game-search-difficulty\" onchange=\"onGameSearchFormChange()\">                     <option value=\"none\">- - -</option>                    <option value=\"beginner\">Těžce podprůměrné</option>                    <option value=\"easy\">Podprůměrné</option>                    <option value=\"medium\">Průměrné</option>                    <option value=\"hard\">Nadprůměrné</option>                    <option value=\"extreme\">Božsky nadprůměrné</option>                    </option>                </select>                <span class=\"icon-chevron-down\"></span>            </div>            <span class=\"text-slate-700 text-sm mt-2 block mb-1\">                Datum poslední změny:            </span>            <input type=\"date\" id=\"game-search-date\" onchange=\"onGameSearchFormChange()\" />            <hr class=\"border-border mt-6\"/>                    </div>        ");
	
            SearchResultListProps search_result_props = {
                .game = props->game,
                .length = props->length,
            };
            char *list = render_search_result_list(&search_result_props);
            fast_strcat(output, list);
            free(list);
        
	fast_strcat(output, "        </div>    <script src=\"/dist/modules/game/main.js\"></script></main>");


    return output;
}
