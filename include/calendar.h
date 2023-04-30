#ifndef CALENDAR_H_INCLUDED
#define CALENDAR_H_INCLUDED

#include "utils.h"

typedef struct
{
    int day;
    int month;
    int year;
} DATE;

typedef struct
{
    BUTTON ** buttons;
    short n_buttons;
} CALENDAR;

act_result calendar(void);

#endif // CALENDAR_H_INCLUDED
