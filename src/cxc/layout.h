#ifndef layout
#define layout

typedef struct {
    
    char *children;

} LayoutProps;

char *render_layout(LayoutProps *props);

#endif
