#include <string.h>
#include <time.h>

#include "calendar.h"
#include "globals.h"

#define MAX_BUTTONS     31 + 4

#define MONTH_PREV  MAX_BUTTONS - 4
#define MONTH_NEXT  MAX_BUTTONS - 3
#define YEAR_PREV   MAX_BUTTONS - 2
#define YEAR_NEXT   MAX_BUTTONS - 1

#define MONTH_TXT_WIDTH 21
#define YEAR_TXT_WIDTH  10

#define WHITESPACE  12

#define DAY_HEIGHT  3
#define DAY_WIDTH   11

#define DAYS_YPOS   7
#define DAYS_XPOS   4

#define DAYS_WIN_HEIGHT (6 * DAY_HEIGHT + 5)
#define DAYS_WIN_WIDTH  (7 * DAY_WIDTH + 6)

DATE date;
const char * month_name[12] = {
    "JANUARY",
    "FEBRUARY",
    "MARCH",
    "APRIL",
    "MAY",
    "JUNE",
    "JULY",
    "AUGUST",
    "SEPTEMBER",
    "OCTOBER",
    "NOVEMBER",
    "DECEMBER"
};

CALENDAR * create_newcalendar(void);
static void destroy_calendar(CALENDAR *);

static void refresh_calendar(CALENDAR *, short n_days, short first_day);

static short get_number_of_days(short month, short year);
static short get_day_number(short day, short month, short year);
static bool is_leap_year(short year);

DATE get_from_calendar(void)
{
    return calendar(true);
}

act_result standalone_calendar(void)
{
    calendar(false);

    return ACT_RETURN;
}

DATE calendar(bool selecting)
{
    CALENDAR * calendar = create_newcalendar();
    BUTTON ** buttons = calendar->buttons;

    time_t c_t = time(NULL);
    struct tm tm = *localtime(&c_t);

    date.day = tm.tm_mday;
    date.month = tm.tm_mon + 1;
    date.year = tm.tm_year + 1900;

    dim_box box = (dim_box){
        .height = 3,
        .width = 3,
        .xpos = (mainwin_width - (MONTH_TXT_WIDTH + 6)) / 2,
        .ypos = 1
    };

    box.xpos = (mainwin_width - DAYS_WIN_WIDTH) / 2 + DAYS_XPOS + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "SUN");
    box.xpos += DAY_WIDTH + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "MON");
    box.xpos += DAY_WIDTH + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "TUE");
    box.xpos += DAY_WIDTH + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "WED");
    box.xpos += DAY_WIDTH + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "THU");
    box.xpos += DAY_WIDTH + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "FRI");
    box.xpos += DAY_WIDTH + 1;
    mvwprintw(mainwin, DAYS_YPOS, box.xpos, "SAT");
    wrefresh(mainwin);

    mvwprintw(mainwin, mainwin_height - 6, 2, "ARROW KEYS: Navigate");
    mvwprintw(mainwin, mainwin_height - 5, 2, "ENTER KEY: Select");
    mvwprintw(mainwin, mainwin_height - 4, 2, "CTRL + Q: Quit/Cancel");

    if (selecting)
        mvwprintw(mainwin, mainwin_height - 3, 2, "- Select a specific day to add a task.");

    WINDOW * dayswin = calendar->dayswin = create_newwin(DAYS_WIN_HEIGHT, DAYS_WIN_WIDTH, DAYS_YPOS + 2, sidebar_width + DAYS_YPOS, -1, 0);

    short n_days = get_number_of_days(date.month, date.year);
    short first_day = get_day_number(1, date.month, date.year);

    wchar_t c;
    WINDOW * tmp_win = dayswin;
    bool in_mainwin = false;
    int b_i = date.day - 1;

    #define highlight_b(i) \
        change_button_style(buttons, tmp_win, calendar->n_buttons, i, buttons[i]->highlight)
    #define reset_b(i) \
        change_button_style(buttons, tmp_win, calendar->n_buttons, i, buttons[i]->style)

    refresh_calendar(calendar, n_days, first_day);

    highlight_b(b_i);

    while (true)
    {
        c = wgetch(tmp_win);

        tmp_win = in_mainwin ? mainwin : dayswin;

        reset_b(b_i);
        switch (c)
        {
            case KEY_LEFT:
                if (in_mainwin)
                {
                    if (b_i > MONTH_PREV)
                        b_i--;
                    else
                        b_i = YEAR_NEXT;
                }
                else
                {
                    if (b_i > 0)
                        b_i--;
                }
                break;
            case KEY_RIGHT:
                if (in_mainwin)
                {
                    if (b_i < YEAR_NEXT)
                        b_i++;
                    else
                        b_i = MONTH_PREV;
                }
                else
                {
                    if (b_i < n_days - 1)
                        b_i++;
                }
                break;
            case KEY_UP:
                if (in_mainwin)
                {
                    in_mainwin = false;
                    tmp_win = dayswin;
                    b_i = n_days - 1;
                }
                else
                {
                    if (b_i > 7)
                        b_i -= 7;
                    else if (b_i > 0)
                        b_i = 0;
                    else
                    {
                        in_mainwin = true;
                        tmp_win = mainwin;
                        b_i = MONTH_PREV;
                    }
                }
                break;
            case KEY_DOWN:
                if (in_mainwin)
                {
                    in_mainwin = false;
                    tmp_win = dayswin;
                    b_i = 0;
                }
                else
                {
                    if (b_i < n_days - 1 - 7)
                        b_i += 7;
                    else if (b_i < n_days - 1)
                        b_i = n_days - 1;
                    else
                    {
                        in_mainwin = true;
                        tmp_win = mainwin;
                        b_i = MONTH_PREV;
                    }
                }
                break;
            case '\n':
                if (in_mainwin)
                {
                    switch (b_i)
                    {
                        case MONTH_PREV:
                            if (date.month > 1)
                                date.month--;
                            else if (date.year > 1602)
                            {
                                date.month = 12;
                                date.year = dec_clamp(date.year, 1601);
                            }
                            break;
                        case MONTH_NEXT:
                            if (date.month < 12)
                                date.month++;
                            else
                            {
                                date.month = 1;
                                date.year++;
                            }
                            break;
                        case YEAR_PREV:
                            date.year = dec_clamp(date.year, 1601);
                            break;
                        case YEAR_NEXT:
                            date.year++;
                            break;
                    }

                    n_days = get_number_of_days(date.month, date.year);
                    first_day = get_day_number(1, date.month, date.year);
                    refresh_calendar(calendar, n_days, first_day);
                }
                else
                {
                    date.day = b_i + 1;
                    if (selecting)
                    {
                        destroy_calendar(calendar);
                        reset_mainwin();
                        return date;
                    }
                }
                break;
            case CTRL('q'):
                destroy_calendar(calendar);
                reset_mainwin();
                date.day = -1;
                return date;

            // Ignore the following keys
            case KEY_IC: // Insert key
            case KEY_DC: // Delete key
            case KEY_HOME: // Home key
            case KEY_END: // End key
            case KEY_NPAGE: // Page down key
            case KEY_PPAGE: // Page up key
                break;

            default: break;
        }
        highlight_b(b_i);

        wrefresh(tmp_win);
    }

    #undef highlight_b
    #undef reset_b

    return date;
}

CALENDAR * create_newcalendar(void)
{
    CALENDAR * calendar = (CALENDAR *)malloc(sizeof(CALENDAR));

    calendar->n_buttons = 31 + 4;
    init_buttons(&calendar->buttons, calendar->n_buttons);

    return calendar;
}

static void destroy_calendar(CALENDAR * calendar)
{
    destroy_win(calendar->dayswin);
    free_arr((void **)calendar->buttons, calendar->n_buttons);
    free(calendar);
}

static void refresh_calendar(CALENDAR * calendar, short n_days, short first_day)
{
    WINDOW * dayswin = calendar->dayswin;
    BUTTON ** buttons = calendar->buttons;

    dim_box box = (dim_box){
        .height = 3,
        .width = 3,
        .xpos = (mainwin_width - (MONTH_TXT_WIDTH + 6)) / 2,
        .ypos = 1
    };

    add_button(mainwin, buttons[MONTH_PREV], "<", box, COLOR_PAIR(CYAN_BG_BLACK), 0);

    wclear_from(mainwin, box.ypos + 1, box.xpos + 3, MONTH_TXT_WIDTH);
    const char * month = month_name[date.month - 1];
    mvwprintw(mainwin, 2, box.xpos + 3 + (MONTH_TXT_WIDTH - strlen(month)) / 2, "%s", month);

    box.xpos += MONTH_TXT_WIDTH + 3;
    add_button(mainwin, buttons[MONTH_NEXT], ">", box, COLOR_PAIR(CYAN_BG_BLACK), 0);

    box.xpos += WHITESPACE + 3;
    add_button(mainwin, buttons[YEAR_PREV], "<", box, COLOR_PAIR(CYAN_BG_BLACK), 0);

    mvwprintw(mainwin, 2, box.xpos + 3 + (YEAR_TXT_WIDTH - 4) / 2, "%d", date.year);

    box.xpos += YEAR_TXT_WIDTH + 3;
    add_button(mainwin, buttons[YEAR_NEXT], ">", box, COLOR_PAIR(CYAN_BG_BLACK), 0);

    werase(dayswin);

    for (int i = 0; i < n_days; i++)
    {
        box = (dim_box){
            .height = DAY_HEIGHT,
            .width = DAY_WIDTH,
            .xpos = (DAY_WIDTH + 1) * ((i + first_day) % 7),
            .ypos = (DAY_HEIGHT + 1) * ((int)((i + first_day) / 7))
        };
        char * str_day = (char *)malloc(3 * sizeof(char));
        itoa(i + 1, str_day, 10);

        add_button(dayswin, buttons[i], str_day, box, 0, 0);
    }

    wrefresh(dayswin);
}

// Gets number of days of a specific month
static short get_number_of_days(short month, short year){
    switch(month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;

        case 2:
            return is_leap_year(year) ? 29 : 28;
   }
   return -1;
}

// Gets the day index (0 - Sat, 1 - Mon, ...)
static short get_day_number(short d, short m, short y)
{
    return (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

// Checks whether the year passed is leap year or not
static bool is_leap_year(short year)
{
    return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
}
