#include "search_result_card.h"

#include <stdlib.h>
#include "../cx.h"


#include <stdlib.h>


char *render_search_result_card(SearchResultCardProps *props)
{
    char *output = calloc(262443+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<div class=\"border-2 border-border border-b-4 rounded-lg p-4\"    data-search-id=\"");
	fast_strcat(output, props->game->id);
	fast_strcat(output, "\">    <h3>");
	fast_strcat(output, props->game->name);
	fast_strcat(output, "</h3>    <p class=\"text-slate-400\">");
	fast_strcat(output, props->game->updated_at);
	fast_strcat(output, "</p></div>");


    return output;
}
