#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "form.h"
#include "login.h"
#include "mainpage.h"
#include "globals.h"

#define WIN_COLS 128
#define WIN_ROWS 42

const char * db_dir = "db/";
const short db_dir_len = 3;
const char * db_ext = ".db";
const short db_ext_len = 3;
const char * note_ext = ".note";
const short note_ext_len = 4;

char * user_dir;


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
    init_pair(GREEN_BG_BLACK, COLOR_BLACK, COLOR_GREEN);
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

    resize_term(WIN_ROWS, WIN_COLS);

    while (true)
    {
        /* INITIATE LOGIN INTERFACE */
        act_result res = init_login();

        if (res == ACT_RETURN) break;

    //    user = (USER *)malloc(sizeof(USER));
    //    user->username = "Test";
    //    user->new_account = false;

        user_dir = (char *)malloc(sizeof(char) * (strlen(db_dir) + strlen(user->username) + 2));
        strcpy(user_dir, db_dir);
        strcat(user_dir, user->username);

        if (user->new_account)
        {
            if (mkdir(user_dir))
            {
                destroy_user();
                free(user_dir);

                printf("error: could not create directory.\n");
                exit(EXIT_FAILURE);
            }
        }

        /* INITIALIZE MAIN PAGE */
        curs_set(0);
        refresh();
        init_main_page();
    }

    endwin();

    return 0;
}
