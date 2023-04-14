#include <stdio.h>
#include <stdlib.h>
#include "form.h"
#include "login.h"

int main()
{
    /* CURSES INIT */
    initscr();
    noecho();
    keypad(stdscr, TRUE);

    /* INITIATE LOGIN INTERFACE */
    init_login();

    endwin();

    return 0;
}
