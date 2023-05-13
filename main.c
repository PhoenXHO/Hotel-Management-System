#include <stdio.h>
#include <stdlib.h>
#include "form.h"
#include "login.h"
#include "mainpage.h"

#define WIN_COLS 128
#define WIN_ROWS 42

void init_colors()
{
    init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);

    init_pair(WHITE_BG_BLACK, COLOR_BLACK, COLOR_WHITE);
    init_pair(BLUE_BG_WHITE, COLOR_WHITE, COLOR_BLUE);
    init_pair(RED_BG_WHITE, COLOR_WHITE, COLOR_RED);
    init_pair(CYAN_BG_WHITE, COLOR_WHITE, COLOR_CYAN);
    init_pair(CYAN_BG_BLACK, COLOR_BLACK, COLOR_CYAN);
    init_pair(RED_BG_BLUE, COLOR_BLUE, COLOR_RED);
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
