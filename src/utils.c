#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "globals.h"

// Clamp a number to a given range
double clamp(double x, double _min, double _max)
{
    double _x = x < _min ? _min : x;
    return _x > _max ? _max : _x;
}
// Clamp and decrement a number
double dec_clamp(double x, double _min)
{
    return x > _min ? x - 1 : _min;
}
// Clamp and increment a number
double inc_clamp(double x, double _max)
{
    return x < _max ? x + 1 : _max;
}

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
    if (!new_arr && new_cap != 0) exit(1);

    return new_arr;
}

// Initialize a field
FIELD * create_newfield(void)
{
    FIELD * field = (FIELD *)malloc(sizeof(FIELD));
    field->line = create_newline();
    field->error = "";

    return field;
}

// Deallocate memory used by a field
void destroy_field(FIELD * field)
{
    destroy_win(field->win);
    free(field->line->buffer);
    free(field->line);
    free(field);
}

// Initialize fields
void init_fields(FIELD *** fields, int n_fields)
{
    *fields = (FIELD **)malloc(n_fields * sizeof(FIELD *));
    for (int i = 0; i < n_fields; i++)
        (*fields)[i] = create_newfield();
}

// Initialize lines
void init_lines(LINE *** lines, int n_lines)
{
    *lines = (LINE **)malloc(n_lines * sizeof(LINE *));
    for (int i = 0; i < n_lines; i++)
        (*lines)[i] = create_newline();
}

// Initialize line
LINE * create_newline(void)
{
    LINE * line = (LINE *)malloc(sizeof(LINE));
    line->capacity = INITIAL_CAP;
    line->buffer = (char *)malloc(line->capacity * sizeof(char));
    line->buffer[0] = '\0';
    line->curs_pos = line->length = line->strstart = 0;

    return line;
}

// Deallocate memory used by a line
void destroy_line(LINE * line)
{
    free(line->buffer);
    free(line);
}

// Initialize buttons
void init_buttons(BUTTON *** buttons, int n_buttons)
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

// Create and initialize a button
void create_newbutton(BUTTON * button, char * content, dim_box box, chtype style, chtype highlight)
{
    button->rows = box.height;

    int len = strlen(content);
    button->cols = len > box.width ? len : box.width;

    button->xpos = box.xpos;
    button->ypos = box.ypos;

    button->content = (char *)malloc((len + 1) * sizeof(char));
    strncpy(button->content, content, len);
    button->content[len] = '\0';

    button->style = style == 0 ? A_BLINK : style;
    button->highlight = highlight == 0 ? A_STANDOUT : highlight;
}

// Deallocate memory used by a button
void destroy_button(BUTTON * button)
{
    free(button->content);
    free(button);
}

// Add button to the window
void add_button(WINDOW * win, BUTTON * button, char * content, dim_box box, chtype style, chtype highlight)
{
    create_newbutton(button, content, box, style, highlight);

    button->xpos = (box.xpos >= 0) ? box.xpos : (win->_maxx - button->cols) / 2;
    button->ypos = (box.ypos >= 0) ? box.ypos : (win->_maxy - button->rows) / 2;

    wattron(win, button->style);
    printb(button, win);
    wattroff(win, button->style);
}

// Change button style
void change_button_style(BUTTON ** buttons, WINDOW * win, short n_buttons, int index, chtype style)
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
void handle_line(WINDOW * win, LINE * line, wchar_t ch, short * curs_pos, short max_size, short line_pos)
{
    short * buff_pos = &line->curs_pos;
    short * strstart = &line->strstart;

    if (isprint(ch))
    {
        // Allocate more memory if necessary
        if (line->length + 1 >= line->capacity)
        {
            line->capacity = GROW_CAPACITY(line->capacity);
            line->buffer = GROW_ARRAY(char, line->buffer, line->capacity);
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
        else if ((*strstart) > 0 && (*curs_pos) > 2)
        {
            // Move the cursor back one character
            (*curs_pos)--;
            // Remove last (duplicate) character from the window
            mvwaddch(win, line_pos, line->length - (*strstart), ' ');
        }
        else if ((*strstart) > 0)
        {
            // Move the whole string to the right
            (*strstart)--;
        }
        else
        {
            // Move the cursor back one character
            (*curs_pos)--;
            // Remove last (duplicate) character from the window
            if (line->length <= max_size)
                mvwaddch(win, line_pos, line->length, ' ');
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

// Clear a line of length `length` on a specific window
void wclear_from(WINDOW * win, short ypos, short xpos, short length)
{
    for (int i = 0; i < length; i++, xpos++)
    {
        wmove(win, ypos, xpos);
        wprintw(win, " ");
        wrefresh(win);
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
            string->capacity = GROW_CAPACITY(string->capacity);
            string->buffer = GROW_ARRAY(char, string->buffer, string->capacity);
        }

        string->buffer[string->length++] = c;
        string->buffer[string->length] = '\0';

        c = fgetc(file);
    }

    string->capacity = string->length + 1;
    string->buffer = SHRINK_ARRAY(char, string->buffer, string->capacity);

    return string;
}

// Free memory allocated for a list of elements
void free_arr(void ** arr, int n_elem)
{
    for (int i = 0; i < n_elem; i++)
        free(arr[i]);
    free(arr);
}

void reset_mainwin(void)
{
    werase(mainwin);
    wattron(mainwin, COLOR_PAIR(CYAN));
    box(mainwin, 0, 0);
    wattroff(mainwin, COLOR_PAIR(CYAN));
    curs_set(0);
    wrefresh(mainwin);
}

act_result __quit__(void) { return ACT_RETURN; }
act_result __continue__(void) { return ACT_CONTINUE; }

char * create_newdir(const char * name, const char * ext)
{
    int dir_len = strlen(user->username) + db_dir_len;
    int len = strlen(name) + dir_len + strlen(ext) + 1;
    char * dir = (char *)malloc(len * sizeof(char));

    strcpy(dir, db_dir);
    strcat(dir, user->username);

    dir[dir_len] = '/';
    dir[dir_len + 1] = '\0';

    strcat(dir, name);
    strcat(dir, ext);

    return dir;
}

bool is_valid_filename(char * name, int length)
{
    for (int i = 0; i < length; i++)
        if (!isalpha(name[i]) && !isdigit(name[i]) && name[i] != '_')
            return false;
    return true;
}

char * dfscans(FILE * file)
{
    if (!file)
        return NULL;

    char * string = (char *)malloc(sizeof(char));

    int len = 0;

    char ch;
    while ((ch = fgetc(file)) != EOF && ch != '\n' && ch != '\0')
    {
        string = GROW_ARRAY(char, string, ++len + 1);

        string[len - 1] = ch;
    }
    string[len] = '\0';

    return string;
}

void fassert(FILE * file, char * file_dir)
{
    if (!file)
    {
        fprintf(stderr, "error: could not open file '%s'.\n", file_dir);
        exit(EXIT_FAILURE);
    }
}

char * strsep(char ** stringp, const char * delim)
{
    char * rv = *stringp;
    if (rv)
    {
        *stringp += strcspn(*stringp, delim);
        if (**stringp)
            *(*stringp)++ = '\0';
        else
            *stringp = 0;
    }

    return rv;
}

void append_offset(char * str, char * source, int offset, int str_length, int source_length)
{
    for (int i = str_length; i < str_length + offset; i++)
        str[i] = ' ';

    for (int i = str_length + offset; i < str_length + offset + source_length; i++)
        str[i] = source[i - (str_length + offset)];

    str[str_length + offset + source_length] = '\0';
}

void append_spaces(char * str, int n_spaces, int length)
{
    for (int i = length; i < length + n_spaces; i++)
        str[i] = ' ';

    str[length + n_spaces] = '\0';
}
