#include "search_result_card.h"

#include <stdlib.h>
#include "../cx.h"


#include <stdlib.h>


char *render_search_result_card(SearchResultCardProps *props)
{
    char *output = calloc(262354+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<div class=\"border-2 border-border border-b-4 rounded-lg p-4\"    data-search-id=\"00000000-0000-4000-8000-000000000000\">    <h3>I use arch btw</h3>    <p class=\"text-slate-400\">11. 12. 2024 11:30</p></div>");


    return output;
}
