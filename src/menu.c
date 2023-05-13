#include "menu.h"

MENU * create_newmenu(dim_box box, short n_buttons)
{
    MENU * menu = (MENU *)malloc(sizeof(MENU *));

    menu->rows = box.height;
    menu->cols = box.width;

    menu->n_buttons = n_buttons;
    init_buttons(&menu->buttons, n_buttons);

    menu->win = create_newwin(box.height, box.width, box.ypos, box.xpos, 1, COLOR_PAIR(CYAN));

    return menu;
}

void destroy_menu(MENU * menu)
{
    destroy_win(menu->win);
    free_arr((void **)menu->buttons, menu->n_buttons);
    free(menu);
}

void handle_menu(MENU * menu)
{
    wchar_t c;
    short b_i = 0;
    act_result result = ACT_CONTINUE;

    BUTTON ** buttons = menu->buttons;

    #define highlight_b(i) \
        change_button_style(buttons, menu->win, menu->n_buttons, i, buttons[i]->highlight)
    #define reset_b(i) \
        change_button_style(buttons, menu->win, menu->n_buttons, i, buttons[i]->style)

    highlight_b(b_i);

    while (true)
    {
        c = wgetch(menu->win);

        reset_b(b_i);
        switch (c)
        {
            case KEY_UP:
                if (b_i > 0)
                    b_i--;
                else
                    b_i = menu->n_buttons - 1;
                break;
            case KEY_DOWN:
                if (b_i < menu->n_buttons - 1)
                    b_i++;
                else
                    b_i = 0;
                break;
            case '\n':
                result = buttons[b_i]->action();
                if (result == ACT_CONTINUE)
                    continue;
                else if (result == ACT_RETURN)
                    return;
                break;

            default: break;
        }
        highlight_b(b_i);

        wrefresh(menu->win);
    }

    #undef highlight_b
    #undef reset_b
}
