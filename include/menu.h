#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "utils.h"

typedef struct
{
    WINDOW * win;
    BUTTON ** buttons;
    int n_buttons;
    short cols;
    short rows;
} MENU;

typedef struct
{
    WINDOW * win;
    BUTTON ** buttons;
    short cols;
    short rows;
} PROMPT;

typedef struct
{
    WINDOW * win;
    BUTTON * button;
    short cols;
    short rows;
} ALERT;

MENU * create_newmenu(dim_box, int n_buttons);
void destroy_menu(MENU *);

PROMPT * create_newprompt(dim_box, const char *);
void destroy_prompt(PROMPT *);

ALERT * create_newalert(dim_box, const char *, chtype highlight);
void destroy_alert(ALERT *);

act_result handle_menu(MENU *);
bool handle_prompt(PROMPT *);
void handle_alert(ALERT *);
void handle_list(WINDOW *, BUTTON ***, int n_buttons, short max_num, func __refresh__, func __update_list__);

#endif // MENU_H_INCLUDED
