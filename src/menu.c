#include "menu.h"
#include "globals.h"

static void print_buttons(WINDOW *, BUTTON **, int n_buttons, short max_num, dim_box);

MENU * create_newmenu(dim_box box, int n_buttons)
{
    MENU * menu = (MENU *)malloc(sizeof(MENU));

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

PROMPT * create_newprompt(dim_box box, const char * message)
{
    PROMPT * prompt = (PROMPT *)malloc(sizeof(PROMPT));

    prompt->rows = 7;
    prompt->cols = box.width;

    init_buttons(&prompt->buttons, 2);

    prompt->win = create_newwin(prompt->rows, box.width, box.ypos, box.xpos, 1, COLOR_PAIR(CYAN));

    mvwprintw(mainwin, prompt->win->_begy + prompt->win->_maxy + 1, prompt->win->_begx - sidebar_width + 2, "ARROWS: Navigate");
    mvwprintw(mainwin, prompt->win->_begy + prompt->win->_maxy + 2, prompt->win->_begx - sidebar_width + 2, "ENTER: Select");
    wrefresh(mainwin);

    mvwprintw(prompt->win, 2, 2, message);

    box = (dim_box){
        .height = 1,
        .width = 10,
        .xpos = (prompt->cols - (10 + 6 + 10)) / 2,
        .ypos = prompt->rows - 3
    };
    add_button(prompt->win, prompt->buttons[0], "Cancel", box, 0, COLOR_PAIR(CYAN_BG_BLACK));
    box.xpos += 10 + 6;
    add_button(prompt->win, prompt->buttons[1], "OK", box, 0, COLOR_PAIR(RED_BG_WHITE));

    return prompt;
}

void destroy_prompt(PROMPT * prompt)
{
    destroy_win(prompt->win);
    free_arr((void **)prompt->buttons, 2);
    free(prompt);
}

ALERT * create_newalert(dim_box box, const char * message, chtype highlight)
{
    ALERT * alert = (ALERT *)malloc(sizeof(ALERT));

    alert->rows = 7;
    alert->cols = box.width;

    alert->button = (BUTTON *)malloc(sizeof(BUTTON));

    alert->win = create_newwin(alert->rows, box.width, box.ypos, box.xpos, 1, COLOR_PAIR(CYAN));

    mvwprintw(alert->win, 2, 2, message);

    box = (dim_box){
        .height = 1,
        .width = 10,
        .xpos = (alert->cols - 10) / 2,
        .ypos = alert->rows - 3
    };
    add_button(alert->win, alert->button, "OK", box, 0, highlight);

    return alert;
}

void destroy_alert(ALERT * alert)
{
    destroy_win(alert->win);
    free(alert->button);
    free(alert);
}

bool handle_prompt(PROMPT * prompt)
{
    short b_i = 0;

    BUTTON ** buttons = prompt->buttons;

    #define highlight_b(i) \
        change_button_style(buttons, prompt->win, 2, i, buttons[i]->highlight)
    #define reset_b(i) \
        change_button_style(buttons, prompt->win, 2, i, buttons[i]->style)

    highlight_b(b_i);

    wchar_t c;
    while (true)
    {
        c = wgetch(prompt->win);

        reset_b(b_i);
        switch (c)
        {
            case KEY_LEFT: case KEY_RIGHT:
                if (b_i == 0)
                    b_i = 1;
                else
                    b_i = 0;
                break;
            case '\n':
                destroy_prompt(prompt);
                if (b_i == 0)
                    return false;
                else
                    return true;

            default: break;
        }
        highlight_b(b_i);

        wrefresh(prompt->win);
    }

    #undef highlight_b
    #undef reset_b
}

void handle_alert(ALERT * alert)
{
    BUTTON * button = alert->button;

    change_button_style(&button, alert->win, 1, 0, button->highlight);
    wrefresh(alert->win);

    wchar_t c;
    while (true)
    {
        c = wgetch(alert->win);

        if (c == '\n')
        {
            destroy_alert(alert);
            break;
        }
    }
}

act_result handle_menu(MENU * menu)
{
    wchar_t c;
    short b_i = 0;

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
            {
                act_result result = buttons[b_i]->action();
                destroy_menu(menu);
                return result;
            }

            default: break;
        }
        highlight_b(b_i);

        wrefresh(menu->win);
    }

    #undef highlight_b
    #undef reset_b
}

void handle_list(WINDOW * win, BUTTON *** buttons, int n_buttons, short max_num, func __refresh__, func __update_list__)
{
    wchar_t c;
    int b_i = 0;
    short y = 1;
    int liststart = 0;

    #define highlight_b(i) \
        change_button_style((*buttons), win, n_buttons, i, (*buttons)[i]->highlight)
    #define reset_b(i) \
        change_button_style((*buttons), win, n_buttons, i, (*buttons)[i]->style)

    dim_box box = {
        .height = (*buttons)[0]->rows,
        .width = (*buttons)[0]->cols,
        .xpos = (*buttons)[0]->xpos,
        .ypos = (*buttons)[0]->ypos
    };

    print_buttons(win, (*buttons), n_buttons, max_num, box);

    highlight_b(b_i);

    while (true)
    {
        if (n_buttons > max_num - 1 && liststart > 0)
            mvwaddch(mainwin, win->_begy + 1, win->_begx - sidebar_width - 3, ACS_UARROW);
        else
            mvwaddch(mainwin, win->_begy + 1, win->_begx - sidebar_width - 3, ' ');

        if (n_buttons > max_num - 1 && liststart < n_buttons - max_num)
            mvwaddch(mainwin, win->_begy + win->_maxy - 3, win->_begx - sidebar_width - 3, ACS_DARROW);
        else
            mvwaddch(mainwin, win->_begy + win->_maxy - 3, win->_begx - sidebar_width - 3, ' ');

        wrefresh(mainwin);

        c = wgetch(win);

        reset_b(b_i);
        switch (c)
        {
            case KEY_UP:
                if (y == 1 && n_buttons > max_num - 1 && liststart > 0)
                {
                    liststart--;
                    werase(win);
                    print_buttons(win, &(*buttons)[liststart], n_buttons - liststart, max_num, box);
                }
                if (y > 1)
                    y--;
                if (b_i > 0)
                    b_i--;
                break;
            case KEY_DOWN:
                if (y == max_num && n_buttons > max_num - 1 && liststart < n_buttons - max_num)
                {
                    liststart++;
                    werase(win);
                    print_buttons(win, &(*buttons)[liststart], n_buttons - liststart, max_num, box);
                }
                if (y < max_num)
                    y++;
                if (b_i < n_buttons - 1)
                    b_i++;
                break;
            case '\n':
            {
                act_result result = ((*buttons)[b_i]->action)(b_i);

                if (result == ACT_UPDATE)
                    __update_list__(buttons, (*buttons)[b_i]->action, (*buttons)[b_i]->highlight, &n_buttons);
                if (b_i >= n_buttons)
                    b_i = n_buttons - 1;
                if (n_buttons > max_num && liststart >= n_buttons - max_num)
                    liststart = n_buttons - max_num;
                if (n_buttons == 0)
                    return;

                __refresh__();
                werase(win);
                print_buttons(win, &(*buttons)[liststart], n_buttons - liststart, max_num, box);
                break;
            }

            case CTRL('q'):
                return;

            default: break;
        }
        highlight_b(b_i);

        wrefresh(win);
    }

    #undef highlight_b
    #undef reset_b
}

static void print_buttons(WINDOW * win, BUTTON ** buttons, int n_buttons, short max_num, dim_box box)
{
    for (int i = 0; i < max_num && i < n_buttons; i++, box.ypos += box.height + 1)
        add_button(win, buttons[i], buttons[i]->content, box, buttons[i]->style, buttons[i]->highlight);
}
