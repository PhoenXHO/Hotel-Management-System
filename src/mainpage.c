#include "mainpage.h"
#include "utils.h"
#include "sidebar.h"
#include "globals.h"

#include "example.h"
#include "notepad.h"
#include "calendar.h"

#define first_button_ypos 8

short mainwin_width;
short mainwin_height;

// Initialize main page
void init_main_page(void)
{
    // Temporary info holder
    dim_box box = {0};
    // Initialize sidebar
    SIDEBAR * sidebar = create_sidebar(4, COLOR_PAIR(BLUE));

    // Button 1
    box = (dim_box){
        .height = 3,
        .width = sidebar->cols - 1,
        .xpos = 0,
        .ypos = first_button_ypos
    };
    add_app(sidebar, box, "App 1", 0, title);
    // Button 2
    box.ypos += 4;
    add_app(sidebar, box, "Notepad", 1, textarea);
    // Button 3
    box.ypos += 4;
    add_app(sidebar, box, "Calendar", 2, calendar);
    // Preferences button
    box.ypos = LINES - 6;
    add_app(sidebar, box, "Preferences", 3, NULL);

    // Create the main window
    mainwin_height = LINES;
    mainwin_width = COLS - sidebar->cols;
    mainwin = create_newwin(mainwin_height, mainwin_width, 0, sidebar->cols, 0, COLOR_PAIR(BLUE));
    wrefresh(mainwin);

    handle_sidebar(sidebar);
}
