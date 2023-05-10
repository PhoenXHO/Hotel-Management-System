#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>
#include <curses.h>
#include "colors.h"

/* UTILITY CONSTANTS */
#define V_DOUBLE_LINE       186 // Vertical double line
#define H_DOUBLE_LINE       205 // Horizontal double line
#define TL_DOUBLE_CORNER    201 // Top left double corner
#define TR_DOUBLE_CORNER    187 // Top right double corner
#define BL_DOUBLE_CORNER    200 // Bottom left double corner
#define BR_DOUBLE_CORNER    188 // Bottom right double corner

/* UTILITY MACROS */
#define GROW_CAPACITY(cap) \
    (cap) * 2
#define GROW_ARRAY(type, arr, cap) \
    (type *)reallocate(arr, GROW_CAPACITY(cap) * sizeof(type))
#define SHRINK_ARRAY(type, arr, len) \
    (type *)reallocate(arr, (len) * sizeof(type))

/* UTILITY STRUCTURES */
// These are values that buttons return when calling their action() function
typedef enum
{
    ACT_CONTINUE,
    ACT_RETURN
} act_result;

// Line structure
typedef struct
{
    char * buffer; // Text buffer
    int length; // Buffer length (without '\0')
    int capacity; // Buffer capacity
    short curs_pos; // Cursor position relative to the buffer
    short strstart; // Index of the first character of the string to print
} LINE;

// Field structure
typedef struct
{
    short xpos; // x position
    short ypos; // y position
    short rows; // Size in rows
    short cols; // Size in cols
    LINE * line; // Line containing text
    WINDOW * win; // Field window
    const char * error; // Error message
    bool hidden;
} FIELD;

// Button structure
typedef act_result (* event) ();
typedef struct
{
    short xpos; // x position
    short ypos; // y position
    short rows; // Button height
    short cols; // Button width
    const char * content; // Text content
    chtype style; // Button default style
    chtype highlight; // Button highlight style
    event action; // Action function to be called
} BUTTON;

// This is a helper structure to reduce number of arguments of some functions
typedef struct
{
    short xpos;
    short ypos;
    short height;
    short width;
} dim_box; // Dimension box to store coordinates and sizes

/* UTILITY FUNCTIONS */
// Clamp a number to a given range
double clamp(double, double _min, double _max);
// Clamp and decrement a number
double dec_clamp(double, double _min);
// Clamp and increment a number
double inc_clamp(double, double _max);

// Function to create a new window
WINDOW * create_newwin(int height, int width, int starty, int startx, int type, chtype colors);
// Function to destroy a window
void destroy_win(WINDOW *);

// Function to print a character
void ref_mvwaddch(WINDOW *, int starty, int startx, wchar_t);

// Function to reallocate memory for an array
void * reallocate(void *, size_t new_cap);
// Allocate memory for n fields
void init_fields(FIELD ***, int n_fields);
// Allocate memory for a line
LINE * create_newline(void);
// Allocate memory for n lines
void init_lines(LINE ***, int n_lines);
// Allocate memory for n buttons
void init_buttons(BUTTON ***, int n_buttons);

// I know these functions have too many arguments... I can't do anything about it...
// Add field to the window
void add_field(WINDOW *, FIELD *, const char * label, dim_box, bool hidden, chtype colors);
// Add button to the window
void add_button(WINDOW *, BUTTON *, const char * content, dim_box, chtype style, chtype highlight);

// Print button
void printb(BUTTON *, WINDOW *);
// Function to change highlight of a button
void change_button_style(BUTTON **, WINDOW *, short n_buttons, short index, chtype style);

// Handle text in a line
void handle_line(WINDOW *, LINE *, wchar_t, short * curs_pos, short max_size);

// Clear a line on the screen
void clear_from(short ypos, short xpos, short length);

// Scan a string dynamically from a stream up to a tab character
LINE * fscans_tab(FILE *);

#endif // UTILS_H_INCLUDED
