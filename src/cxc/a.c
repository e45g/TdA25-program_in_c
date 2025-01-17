#include "a.h"

#include <stdlib.h>
#include "../cx.h"





char *render_a(AProps *props)
{
    char *output = calloc(262243+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<html>...");
	props->x = 'A';
	fast_strcat(output, "<p class=\"...\">");
	fast_strcat(output, a);
	fast_strcat(output, "</p></html>");


    return output;
}
