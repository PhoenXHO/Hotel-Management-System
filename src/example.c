#include <stdbool.h>
#include "example.h"

const char * text[] = {
    " _       __     __",
    "| |     / /__  / /________  ____ ___  ___",
    "| | /| / / _ \\/ / ___/ __ \\/ __ `__ \\/ _ \\",
    "| |/ |/ /  __/ / /__/ /_/ / / / / / /  __/",
    "|__/|__/\\___/_/\\___/\\____/_/ /_/ /_/\\___/"
};

act_result title(void)
{
    // Printing some text
    int y = (LINES - 6) / 2 - 3,
        x = (mainwin->_maxx - 42) / 2;
    mvwprintw(mainwin, y    , x, text[0]);
    mvwprintw(mainwin, y + 1, x, text[1]);
    mvwprintw(mainwin, y + 2, x, text[2]);
    mvwprintw(mainwin, y + 3, x, text[3]);
    mvwprintw(mainwin, y + 4, x, text[4]);

    wrefresh(mainwin);

    // Printing buttons
    BUTTON ** buttons;
    short n_buttons = 2;
    init_buttons(&buttons, n_buttons);

    // Negative xpos/ypos to center button horizontally/vertically
    dim_box box = {
        .width = 20,
        .height = 3,
        .xpos = -1,
        .ypos = 20
    };
    add_button(mainwin, buttons[0], "This is a button", box);

    // Change highlight color of first button
    buttons[0]->highlight = COLOR_PAIR(BLUE_BG_WHITE);

    box.width = 24;
    box.ypos = 24;
    add_button(mainwin, buttons[1], "This is another button", box);

    // To select buttons
    wchar_t ch;
    int index = 0;

    #define highlight_b(i) \
        change_button_style(buttons, mainwin, n_buttons, i, buttons[i]->highlight)
    #define reset_b(i) \
        change_button_style(buttons, mainwin, n_buttons, i, buttons[0]->style)

    // Highlight the first button
    highlight_b(0);

    while (true)
    {
        ch = wgetch(mainwin);

        reset_b(index);

        switch (ch)
        {
            case KEY_UP:
                if (index > 0)
                    index--;
                else
                    index = n_buttons - 1;
                break;
            case KEY_DOWN:
                if (index < n_buttons - 1)
                    index++;
                else
                    index = 0;
                break;
            // Quit when pressing '`'
            case '`': return ACT_CONTINUE;
            default: break;
        }

        highlight_b(index);
    }

    #undef highlight_b
    #undef reset_b

    return ACT_CONTINUE;
}
