#include <stdbool.h>
#include <string.h>
#include "sidebar.h"
#include "globals.h"

WINDOW * mainwin;

static SIDEBAR * new_sidebar(chtype colors);

// Create a sidebar
SIDEBAR * create_sidebar(short n_buttons, chtype colors)
{
    // Init sidebar
    SIDEBAR * sidebar = new_sidebar(colors);

    sidebar->cols = sidebar_width;

    sidebar->buttons = NULL;

    // Allocate memory for this number of buttons
    sidebar->n_buttons = n_buttons;
    init_buttons(&sidebar->buttons, sidebar->n_buttons);

    // Print title
    mvwprintw(sidebar->win, 2, (sidebar_width - (10 + strlen(user->username))) / 2, "Welcome, %s!", user->username);
    wrefresh(sidebar->win);

    return sidebar;
}

// Deallocate memory used by the sidebar
void destroy_sidebar(void)
{
    destroy_win(sidebar->win);
    free_arr((void **)sidebar->buttons, sidebar->n_buttons);
    free(sidebar);
}

// Handle sidebar
void handle_sidebar(SIDEBAR * sidebar)
{
    wchar_t ch;
    int i = 0;

    // Macros to highlight/reset a button easily
    #define highlight_b(i) \
        change_button_style(sidebar->buttons, sidebar->win, sidebar->n_buttons, i, sidebar->buttons[i]->highlight)
    #define reset_b(i) \
        change_button_style(sidebar->buttons, sidebar->win, sidebar->n_buttons, i, sidebar->buttons[i]->style)

    // Highlight the first button
    highlight_b(0);

    while (true)
    {
        ch = wgetch(sidebar->win);

        reset_b(i);
        switch (ch)
        {
            case '\n':
                ((d_event)sidebar->buttons[i]->action)();
                if (mainwin) reset_mainwin();
                else return;
                break;
            case KEY_DOWN:
                if (i < sidebar->n_buttons - 1)
                    i++;
                else
                    i = 0;
                break;
            case KEY_UP:
                if (i > 0)
                    i--;
                else
                    i = sidebar->n_buttons - 1;
                break;
            default: break;
        }
        highlight_b(i);
    }

    #undef highlight_b
    #undef reset_b
}

// Create a new empty sidebar
static SIDEBAR * new_sidebar(chtype colors)
{
    // Allocate memory for the sidebar
    SIDEBAR * sidebar = (SIDEBAR *)malloc(sizeof(SIDEBAR));
    // Init the sidebar window
    sidebar->win = create_newwin(LINES, sidebar_width, 0, 0, -1, 0);
    // Move the cursor to the rightmost side of the sidebar
    wmove(sidebar->win, 0, sidebar_width - 1);
    // Draw a right border at the location of the cursor
    wattron(sidebar->win, colors);
    wvline(sidebar->win, V_DOUBLE_LINE, LINES);
    wattroff(sidebar->win, colors);

    wrefresh(sidebar->win);

    return sidebar;
}

// Link an app to a sidebar
void add_app(SIDEBAR * sidebar, dim_box box, char * title, short index, event action, bool primary)
{
    add_button(sidebar->win,
               sidebar->buttons[index],
               title,
               box,
               COLOR_PAIR(WHITE_BG_BLACK),
               primary ? COLOR_PAIR(CYAN_BG_BLACK) : COLOR_PAIR(RED_BG_WHITE));
    sidebar->buttons[index]->action = action;
}
