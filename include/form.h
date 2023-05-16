#ifndef FORM_H_INCLUDED
#define FORM_H_INCLUDED

#include "utils.h"

#define form_height 27
#define form_width 54

// Form structure
typedef struct
{
    short rows; // Size in rows
    short cols; // Size in cols
    short n_fields; // Number of fields
    short n_buttons; // Number of buttons
    FIELD ** fields; // Array of form fields
    BUTTON ** buttons; // Array of buttons
    WINDOW * win; // Form window
} FORM;

FORM * new_form(int height, int width, int starty, int startx, int type, chtype colors);
void destroy_form(FORM *);

void fld_printerr(FIELD *);
void printerr(FORM *);
void fld_clrerr(FIELD *);
void clrerr(FORM *);
void reseterr(FORM *);

#endif // FORM_H_INCLUDED
