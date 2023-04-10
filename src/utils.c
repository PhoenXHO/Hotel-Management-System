#include "utils.h"

// Create a new window at the specified position
WINDOW * create_newwin(int height, int width, int starty, int startx, int type)
{
    // Create a new window object
    WINDOW * win = newwin(height, width, starty, startx);
    // Create a box around the window
    if (type == 0)
        // 0, 0 gives default characters for vertical and horizontal lines
        box(win, 0, 0);
    else
        // Double border
        wborder(win,
                186, // left
                186, // right
                205, // top
                205, // bottom
                201, // top left
                187, // top right
                200, // bottom left
                188  // bottom right
                );
    keypad(win, TRUE);
    wrefresh(win);
    return win;
}

// Destroy a window
void destroy_win(WINDOW * win)
{
    // Clear borders
    wborder(win,
            ' ', // left
            ' ', // right
            ' ', // top
            ' ', // bottom
            ' ', // top left
            ' ', // top right
            ' ', // bottom left
            ' '  // bottom right
            );
    wrefresh(win);
    // Deallocate memory designated to the window
    delwin(win);
}

void ref_mvwaddch(WINDOW * win, int starty, int startx, wchar_t ch)
{
    mvwaddch(win, starty, startx, ch);
    wrefresh(win);
}

void * reallocate(void * arr, size_t new_cap)
{
    void * new_arr = realloc(arr, new_cap);
    if (!new_arr) exit(1);

    return new_arr;
}
