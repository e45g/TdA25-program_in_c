#include "search_result_list.h"

#include <stdlib.h>
#include "../cx.h"


#include <stdlib.h>
#include "search_result_card.h"


char *render_search_result_list(SearchResultListProps *props)
{
    char *output = calloc(262530+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<div class=\"flex gap-4 w-full flex-col items-stretch\" id=\"game-search-results\">    ");
	
        for (int i = 0; i < 3; i++) {
            SearchResultCardProps card_props = {
                .x = i,
            };
            char* card_str = render_search_result_card(&card_props);
            fast_strcat(output, card_str);
            free(card_str);
        }
    
	fast_strcat(output, "</div>");


    return output;
}
