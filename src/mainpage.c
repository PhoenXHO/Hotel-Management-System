#include "mainpage.h"
#include "utils.h"
#include "sidebar.h"
#include "globals.h"

#include "loginhandler.h"
#include "login.h"
#include "calendar.h"
#include "todo.h"
#include "notepad.h"

#define first_button_ypos 8

SIDEBAR * sidebar;

short mainwin_width;
short mainwin_height;

act_result logout(void);

// Initialize main page
void init_main_page(void)
{
    todo_called = false;
    notepad_called = false;

    // Temporary info holder
    dim_box box = {0};
    // Initialize sidebar
    sidebar = create_sidebar(4, COLOR_PAIR(CYAN));

    // Button 1
    box = (dim_box){
        .height = 3,
        .width = sidebar->cols - 1,
        .xpos = 0,
        .ypos = first_button_ypos
    };
    add_app(sidebar, box, "To-do List", 0, todo, true);
    // Button 2
    box.ypos += 4;
    add_app(sidebar, box, "Calendar", 1, standalone_calendar, true);
    // Button 3
    box.ypos += 4;
    add_app(sidebar, box, "Notepad", 2, notepad, true);
    // Log out button
    box.ypos = LINES - 6;
    add_app(sidebar, box, "Log out", 3, logout, false);

    // Create the main window
    mainwin_height = LINES;
    mainwin_width = COLS - sidebar->cols;
    mainwin = create_newwin(mainwin_height, mainwin_width, 0, sidebar->cols, 0, COLOR_PAIR(CYAN));
    wrefresh(mainwin);

    handle_sidebar(sidebar);
}

act_result logout(void)
{
    destroy_user();
    destroy_win(mainwin);
    mainwin = NULL;
    destroy_sidebar();
    destroy_todolist();
    destroy_notelist();

    return ACT_RETURN;
}
