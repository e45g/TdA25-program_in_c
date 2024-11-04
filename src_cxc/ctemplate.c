#include "%%NAME%%.h"
#include "cx.h"
#include <stdlib.h>
#include <string.h>
%%PREPEND%%
char *%%FUNC_NAME%%(%%PROPS_NAME%% *props __attribute__((unused))) {2
    char *output = calloc(%%RESPONSE_SIZE%%+1, sizeof(char));

    %%CODE%%

    return output;
}
