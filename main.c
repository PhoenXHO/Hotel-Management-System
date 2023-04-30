#include <stdio.h>
#include <stdlib.h>
#include "form.h"
#include "login.h"
#include "mainpage.h"

#define WIN_COLS 128
#define WIN_ROWS 50

void init_colors()
{
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(BLUE_BG_WHITE, COLOR_WHITE, COLOR_BLUE);
}

int main()
{
    /* CURSES INIT */
    initscr();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    init_colors();

    /* INITIATE LOGIN INTERFACE */
    resize_term(WIN_ROWS, WIN_COLS);
    //init_login();

    /* INITIALIZE MAIN PAGE */
    curs_set(0);
    refresh();
    init_main_page();

    endwin();

    return 0;
}
