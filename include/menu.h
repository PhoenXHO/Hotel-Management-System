#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "utils.h"

typedef struct
{
    WINDOW * win;
    BUTTON ** buttons;
    short n_buttons;
    short cols;
    short rows;
} MENU;

MENU * create_newmenu(dim_box, short n_buttons);
void destroy_menu(MENU *);

void handle_menu(MENU *);

#endif // MENU_H_INCLUDED
