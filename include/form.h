#ifndef FORM_H_INCLUDED
#define FORM_H_INCLUDED

#include <stdbool.h>
#include "utils.h"

#define form_height 27
#define form_width 54

// Field structure
typedef struct
{
    short rows; // Size in rows
    short cols; // Size in cols
    int cur_pos; // Cursor position relative to the buffer
    char * buffer; // Text buffer
    int length; // Buffer length (without '\0')
    int capacity; // Buffer capacity
    WINDOW * win; // Field window
    bool hidden;
} FIELD;

// Button structure
typedef struct
{
    short xpos; // x position
    short ypos; // y position
    const char * content; // Text content
    chtype style; // Button default style
    chtype highlight; // Button highlight style
} BUTTON;

// Form structure
typedef struct
{
    short rows; // Size in rows
    short cols; // Size in cols
    short n_fields; // Number of fields
    short n_buttons; // Number of buttons
    FIELD ** fields; // Array of form fields
    FIELD * active_f; // Active field
    BUTTON ** buttons; // Array of buttons
    BUTTON * active_b; // Active button
    WINDOW * win; // Form window
} FORM;

FORM * create_loginform(void);
FORM * create_registrform(void);
void destroy_form(FORM *);
void highlight_b(FORM *, short index);
void reset_buttons(FORM *);

#endif // FORM_H_INCLUDED
