#include "layout.h"

#include <stdlib.h>
#include <string.h>
#include "../cx.h"


#include <stdlib.h>


char *render_layout(LayoutProps *props)
{
    char *output = calloc(263366+1, sizeof(char));
    if(!output) return NULL;

    	fast_strcat(output, "<!DOCTYPE html><html lang=\"en\"><head>    <meta charset=\"UTF-8\">    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">    <title>Document</title>    <link rel=\"stylesheet\" href=\"/style.css\" />    <link rel=\"stylesheet\" href=\"/icon/lucide.css\" />    <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\" />    <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin />    <link href=\"https://fonts.googleapis.com/css2?family=Dosis:wght@500&display=swap\" rel=\"stylesheet\" /></head><body class=\"flex flex-col h-screen\">    <nav class=\"w-full border-b-2 border-border bg-white p-4 flex items-center\">        <a href=\"/\" class=\"text-xl font-bold\">            Archie-chan        </a>        ");
	
            if (props->page == 1) {
                fast_strcat(output, "<button class=\"ml-auto\" id=\"game-search-menu-trigger\" data-state=\"closed\"> \
                    <span class=\"icon-arrow-left\"></span> \
                    <span class=\"icon-arrow-right\"></span> \
                </button>");
            }
        
	fast_strcat(output, "    </nav>    ");
	fast_strcat(output, props->children);
	fast_strcat(output, "    <script src=\"/dist/main.js\"></script></body></html>");


    return output;
}
