#ifndef CALENDAR_H_INCLUDED
#define CALENDAR_H_INCLUDED

#include "utils.h"

typedef struct
{
    BUTTON ** buttons;
    short n_buttons;
    WINDOW * dayswin;
} CALENDAR;

DATE get_from_calendar(void);
act_result standalone_calendar(void);
DATE calendar(bool selecting);

#endif // CALENDAR_H_INCLUDED
