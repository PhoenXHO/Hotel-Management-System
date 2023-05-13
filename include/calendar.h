#ifndef CALENDAR_H_INCLUDED
#define CALENDAR_H_INCLUDED

#include "utils.h"

typedef struct
{
    short day;
    short month;
    short year;
} DATE;

typedef struct
{
    BUTTON ** buttons;
    short n_buttons;
    WINDOW * dayswin;
} CALENDAR;

act_result calendar(void);

#endif // CALENDAR_H_INCLUDED
