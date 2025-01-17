#ifndef layout
#define layout
#include "../game.h"

typedef struct {
    
    char *children;
    int page;

} LayoutProps;

char *render_layout(LayoutProps *props);

#endif
