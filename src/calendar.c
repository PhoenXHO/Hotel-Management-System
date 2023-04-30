#include <string.h>

#include "calendar.h"
#include "globals.h"

#define MONTH_PREV  42
#define MONTH_NEXT  43
#define YEAR_PREV   44
#define YEAR_NEXT   45

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
int get_number_of_days(int month, int year);
int get_day_number(int day,int mon,int year);
int check_leapYear(int year);

act_result calendar(void)
{
    CALENDAR * calendar = create_newcalendar();
    BUTTON ** buttons = calendar->buttons;

    date.month = 12;
    date.year = 2023;

    dim_box box = (dim_box){
        .height = 3,
        .width = 3,
        .xpos = (mainwin_width - (MONTH_TXT_WIDTH + 6)) / 2,
        .ypos = 1
    };
    add_button(mainwin, buttons[MONTH_PREV], "<", box, COLOR_PAIR(BLUE_BG_WHITE), 0);

    const char * month = month_name[date.month - 1];
    mvwprintw(mainwin, 2, box.xpos + 3 + (MONTH_TXT_WIDTH - strlen(month)) / 2, "%s", month);

    box.xpos += MONTH_TXT_WIDTH + 3;
    add_button(mainwin, buttons[MONTH_NEXT], ">", box, COLOR_PAIR(BLUE_BG_WHITE), 0);

    box.xpos += WHITESPACE + 3;
    add_button(mainwin, buttons[YEAR_PREV], "<", box, COLOR_PAIR(BLUE_BG_WHITE), 0);

    mvwprintw(mainwin, 2, box.xpos + 3 + (YEAR_TXT_WIDTH - 4) / 2, "%d", date.year);

    box.xpos += YEAR_TXT_WIDTH + 3;
    add_button(mainwin, buttons[YEAR_NEXT], ">", box, COLOR_PAIR(BLUE_BG_WHITE), 0);

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

    WINDOW * dayswin = create_newwin(DAYS_WIN_HEIGHT, DAYS_WIN_WIDTH, DAYS_YPOS + 2, sidebar_width + DAYS_YPOS, -1, 0);

    int n_days = get_number_of_days(date.month, date.year);
    for (int i = get_day_number(1, date.month, date.year), count = 1; count <= n_days; i++, count++)
    {
        box = (dim_box){
            .height = DAY_HEIGHT,
            .width = DAY_WIDTH,
            .xpos = (DAY_WIDTH + 1) * (i % 7),
            .ypos = (DAY_HEIGHT + 1) * ((int)(i / 7))
        };
        char str_day[3];
        itoa(count, str_day, 10);

        add_button(dayswin, buttons[i], str_day, box, 0, 0);
        wrefresh(dayswin);
    }

    wrefresh(mainwin);

    getch();

    return ACT_RETURN;
}

CALENDAR * create_newcalendar(void)
{
    CALENDAR * calendar = (CALENDAR *)malloc(sizeof(CALENDAR));

    calendar->n_buttons = 46;
    init_buttons(&calendar->buttons, calendar->n_buttons);

    return calendar;
}

int get_number_of_days(int month, int year){
   switch(month) {
      case 1 : return(31);
      case 2 : if(check_leapYear(year)==1)
		 return(29);
	       else
		 return(28);
      case 3 : return(31);
      case 4 : return(30);
      case 5 : return(31);
      case 6 : return(30);
      case 7 : return(31);
      case 8 : return(31);
      case 9 : return(30);
      case 10: return(31);
      case 11: return(30);
      case 12: return(31);
   }
   return -1;
}

int get_day_number(int day,int mon,int year){
    int res = 0, t1, t2, y = year;
    year = year - 1600;
    while(year >= 100){
        res = res + 5;
        year = year - 100;
    }
    res = (res % 7);
    t1 = ((year - 1) / 4);
    t2 = (year-1)-t1;
    t1 = (t1*2)+t2;
    t1 = (t1%7);
    res = res + t1;
    res = res%7;
    t2 = 0;
    for(t1 = 1;t1 < mon; t1++){
        t2 += get_number_of_days(t1,y);
    }
    t2 = t2 + day;
    t2 = t2 % 7;
    res = res + t2;
    res = res % 7;
    if(y > 2000)
        res = res + 1;
    res = res % 7;
    return res;
}

int check_leapYear(int year){ //checks whether the year passed is leap year or not
    if(year % 400 == 0 || (year % 100!=0 && year % 4 ==0))
       return 1;
    return 0;
}
