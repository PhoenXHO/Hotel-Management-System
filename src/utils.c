#include <string.h>
#include <ctype.h>
#include "utils.h"

#define INITIAL_CAP 120

// Create a new window at the specified position
WINDOW * create_newwin(int height, int width, int starty, int startx, int type, chtype colors)
{
    // Create a new window object
    WINDOW * win = newwin(height, width, starty, startx);
    // Create a box around the window
    wattron(win, colors);
    if (type == 0)
        // 0, 0 gives default characters for vertical and horizontal lines
        box(win, 0, 0);
    else if (type == 1)
        // Double border
        wborder(win,
                V_DOUBLE_LINE,      // left
                V_DOUBLE_LINE,      // right
                H_DOUBLE_LINE,      // top
                H_DOUBLE_LINE,      // bottom
                TL_DOUBLE_CORNER,   // top left
                TR_DOUBLE_CORNER,   // top right
                BL_DOUBLE_CORNER,   // bottom left
                BR_DOUBLE_CORNER    // bottom right
                );
    wattroff(win, colors);

    keypad(win, TRUE);
    wrefresh(win);

    return win;
}

// Destroy a window
void destroy_win(WINDOW * win)
{
    // Clear window
    wclear(win);
    wrefresh(win);
    // Deallocate memory designated to the window
    delwin(win);
}

// Print a character to a window
void ref_mvwaddch(WINDOW * win, int starty, int startx, wchar_t ch)
{
    mvwaddch(win, starty, startx, ch);
    wrefresh(win);
}

// Reallocate more memory for an array
void * reallocate(void * arr, size_t new_cap)
{
    void * new_arr = realloc(arr, new_cap);
    if (!new_arr) exit(1);

    return new_arr;
}

// Initialize fields
void init_fields(FIELD *** fields, short n_fields)
{
    *fields = (FIELD **)malloc(n_fields * sizeof(FIELD *));
    for (int i = 0; i < n_fields; i++)
    {
        (*fields)[i] = (FIELD *)malloc(sizeof(FIELD));
        (*fields)[i]->line = (LINE *)malloc(sizeof(LINE));
        (*fields)[i]->line->capacity = INITIAL_CAP;
        (*fields)[i]->line->length = 0;
        (*fields)[i]->line->buffer = (char *)malloc((*fields)[i]->line->capacity * sizeof(char));
        (*fields)[i]->line->buffer[0] = '\0';
        (*fields)[i]->line->curs_pos = 0;
        (*fields)[i]->line->strstart = 0;
        (*fields)[i]->error = "";
    }
}

// Initialize buttons
void init_buttons(BUTTON *** buttons, short n_buttons)
{
    *buttons = (BUTTON **)malloc(n_buttons * sizeof(BUTTON *));
    for (int i = 0; i < n_buttons; i++)
        (*buttons)[i] = (BUTTON *)malloc(sizeof(BUTTON));
}

// Print button
void printb(BUTTON * button, WINDOW * win)
{
    int len = strlen(button->content);
    short xpos = button->xpos;
    short ypos = button->ypos;
    short rows = button->rows;
    short cols = button->cols;

    for (int i = 0; i < button->rows; i++)
    {
        for (int j = 0; j < button->cols; j++)
        {
            if (i == rows / 2 && j == (cols - len) / 2)
            {
                mvwprintw(win, ypos + i, xpos + j, button->content);
                j += len - 1;
            }
            else
                mvwaddch(win, ypos + i, xpos + j, ' ');

            wrefresh(win);
        }
    }

    wrefresh(win);
}

// Add field to the window
void add_field(WINDOW * win, FIELD * field, const char * label, dim_box box, bool hidden, chtype colors)
{
    field->rows = box.height;
    field->cols = box.width;

    field->xpos = (box.xpos >= 0) ? box.xpos : (win->_maxx - field->cols) / 2;
    field->ypos = (box.ypos >= 0) ? box.ypos : (win->_maxy - field->rows) / 2;

    field->win = create_newwin(box.height, box.width, field->ypos, field->xpos, 0, colors);
    field->hidden = hidden;

    mvwprintw(field->win, 0, 1, label);
    wrefresh(field->win);
}

// Add button to the window
void add_button(WINDOW * win, BUTTON * button, const char * content, dim_box box)
{
    button->rows = box.height;

    int len = strlen(content);
    button->cols = len > box.width ? len : box.width;

    button->xpos = (box.xpos >= 0) ? box.xpos : (win->_maxx - button->cols) / 2;
    button->ypos = (box.ypos >= 0) ? box.ypos : (win->_maxy - button->rows) / 2;

    button->content = content;
    button->style = A_BLINK;
    button->highlight = A_STANDOUT;

    wattron(win, button->style);
    printb(button, win);
    wattroff(win, button->style);
}

// Change button style
void change_button_style(BUTTON ** buttons, WINDOW * win, short n_buttons, short index, chtype style)
{
    for (int i = 0; i < n_buttons; i++)
        if (i == index)
        {
            wattron(win, style);
            printb(buttons[i], win);
            wattroff(win, style);
        }

    wrefresh(win);
}

// Hanlde line
void handle_line(WINDOW * win, LINE * line, wchar_t ch, short * curs_pos, short max_size)
{
    short * buff_pos = &line->curs_pos;
    short * strstart = &line->strstart;

    if (isprint(ch))
    {
        // Allocate more memory if necessary
        if (line->length + 1 >= line->capacity)
        {
            line->buffer = GROW_ARRAY(char, line->buffer, line->capacity);
            line->capacity = GROW_CAPACITY(line->capacity);
        }

        if (line->length++ == 0) // If the field is empty
        {
            // Simply add the character to the beginning
            line->buffer[0] = ch;
            line->buffer[1] = '\0';
        }
        else
        {
            // Move the string from position (buff_pos) one character to the right
            //      to make room for the new character
            memmove(&line->buffer[(*buff_pos) + 1], &line->buffer[(*buff_pos)], line->length - (*buff_pos) + 1);
            // Insert the new character
            line->buffer[(*buff_pos)] = ch;
        }

        // Order...
        if (line->length > max_size && (*curs_pos) == max_size + 1)
            (*strstart)++;
        if ((*curs_pos) <= max_size)
            (*curs_pos)++;

        (*buff_pos)++;
    }
    // If the user hits a backspace
    else if ((ch == KEY_BACKSPACE || ch == '\b' || ch == 127) && line->length > 0 && (*buff_pos) > 0)
    {
        if (line->length > max_size && (*strstart) > 0)
        {
            // Move the whole string to the right
            (*strstart)--;
            // Remove last (duplicate) character from the window
            mvwaddch(win, 1, max_size, ' ');
        }
        else
        {
            // Move the cursor back one character
            (*curs_pos)--;
            // Remove last (duplicate) character from the window
            if (line->length <= max_size)
                mvwaddch(win, 1, line->length, ' ');
        }

        // Move the string from position (x) one character to the left
        memmove(&line->buffer[(*buff_pos) - 1], &line->buffer[(*buff_pos)], line->length - (*buff_pos) + 1);

        line->length--;
        (*buff_pos)--;
    }
}

// Clear a line of length `length`
void clear_from(short ypos, short xpos, short length)
{
    for (int i = 0; i < length; i++, xpos++)
    {
        move(ypos, xpos);
        printw(" ");
    }
}

// Scan a string dynamically from a stream up to a tab character
LINE * fscans_tab(FILE * file)
{
    LINE * string = (LINE *)malloc(sizeof(LINE));

    string->length = 0;
    string->capacity = INITIAL_CAP;

    string->buffer = (char *)malloc(INITIAL_CAP * sizeof(char));
    string->buffer[0] = '\0';

    char c = fgetc(file);
    while (c != '\t' && c != '\n' && c != '\0' && c != EOF)
    {
        if (string->length + 1 >= string->capacity)
        {
            string->buffer = GROW_ARRAY(char, string->buffer, string->capacity);
            string->capacity = GROW_CAPACITY(string->capacity);
        }

        string->buffer[string->length++] = c;
        string->buffer[string->length] = '\0';

        c = fgetc(file);
    }

    string->capacity = string->length + 1;
    string->buffer = SHRINK_ARRAY(char, string->buffer, string->capacity);

    return string;
}
