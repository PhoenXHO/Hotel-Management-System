#ifndef SIDEBAR_H_INCLUDED
#define SIDEBAR_H_INCLUDED

#include "utils.h"

// Sidebar structure
typedef struct
{
    short cols;
    short n_buttons;
    BUTTON ** buttons; // Array of buttons
    WINDOW * win; // Sidebar window
} SIDEBAR;

// Create a new sidebar
SIDEBAR * create_sidebar(short n_buttons, chtype colors);
// Deallocate memory used by the sidebar
void destroy_sidebar(void);
// Handle sidebar (user input)
void handle_sidebar(SIDEBAR *);
// Link an app to the sidebar
void add_app(SIDEBAR *, dim_box, char * title, short index, event, bool primary);

#endif // SIDEBAR_H_INCLUDED
