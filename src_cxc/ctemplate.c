#include "%%NAME%%.h"

#include <stdlib.h>
#include "cx.h"

%%PREPEND%%
char *%%FUNC_NAME%%(%%PROPS_NAME%% *props __attribute__((unused))) {
    char *output = calloc(%%RESPONSE_SIZE%%+1, sizeof(char));
    if(!output) return NULL;

    %%CODE%%

    return output;
}
