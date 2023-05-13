#include <string.h>
#include <math.h>
#include "notepad.h"
#include "globals.h"

TEXTAREA * textarea;

void init_textarea(void);
void destroy_textarea(void);

static void print_lines(short starty, int from, int to, short max_width);
static void append_string(LINE *, char *, int length);
static void insert_line(int at);
// Deallocate memory reseved for a line
static void delete_line(int at);

act_result notepad(void)
{
    init_textarea();

    int line_i = 0; // Line index
    short x = 1; // Cursor x position relative to `mainwin`
    short y = 1; // Cursor y position relative to `mainwin`
    const short max_width = mainwin_width - 3;
    const short max_height = mainwin_height - 7;
    bool edited = false;

    // Show the cursor if hidden, then move it to the beginning of the first field
    curs_set(1);
    wmove(mainwin, 1, 1);
    wrefresh(mainwin);

    wchar_t ch;
    while (true)
    {
        ch = wgetch(mainwin);

        LINE * line = textarea->lines[line_i];
        switch (ch)
        {
            case KEY_LEFT:
                if (line->length >= max_width && x == 1 && textarea->strstart > 0)
                {
                    textarea->strstart--;
                    line->strstart--;
                    if (textarea->n_lines > max_height)
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    else
                        print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                }
                if (x == 1 && line_i > 0 && line->strstart == 0)
                {
                    if (y > 1)
                        y--;
                    line_i--;
                    line = textarea->lines[line_i];
                    x = line->length + 1;
                    line->curs_pos = line->length;
                    break;
                }
                if (x > 1)
                    x--;
                if (line->curs_pos > 0)
                    line->curs_pos--;
                break;
            case KEY_RIGHT:
                if (line->length >= max_width && x == max_width + 1 && line->curs_pos < line->length)
                {
                    textarea->strstart++;
                    line->strstart++;
                    if (textarea->n_lines > max_height)
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    else
                        print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                }
                if (line->curs_pos == line->length && line_i < textarea->n_lines - 1)
                {
                    if (y < textarea->n_lines)
                        y++;
                    line_i++;
                    line = textarea->lines[line_i];
                    x = 1;
                    line->curs_pos = 0;
                    break;
                }
                if (x < line->length + 1 && x <= max_width)
                    x++;
                if (line->curs_pos < line->length)
                    line->curs_pos++;
                break;
            case KEY_UP:
            {
                short old_b_pos = line->curs_pos;
                line = textarea->lines[line_i - 1];

                if (textarea->strstart > 0 && line->length <= old_b_pos)
                {
                    if (textarea->strstart > line->length)
                    {
                        textarea->strstart = clamp(textarea->strstart - max_width, 0, max_width);
                        x = max_width + 1;
                    }

                    if (textarea->n_lines > max_height)
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    else
                        print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                }
                if (line_i > 0)
                {
                    line_i--;
                    if (textarea->n_lines > max_height && y == 1)
                    {
                        textarea->linestart--;
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    }
                }

                if (y > 1)
                    y--;
                line = textarea->lines[line_i];
                x = (x < (line->length + 1)) ? x : (line->length + 1);
                line->curs_pos = (old_b_pos < line->length) ? old_b_pos : line->length;
                break;
            }
            case KEY_DOWN:
            {
                if (line_i < textarea->n_lines - 1)
                {
                    line_i++;
                    if (line_i > max_height && y == max_height + 1)
                    {
                        textarea->linestart++;
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    }
                }
                if (y < max_height + 1 && y < textarea->n_lines)
                    y++;

                short old_b_pos = line->curs_pos;
                line = textarea->lines[line_i];
                x = (x < (line->length + 1)) ? x : (line->length + 1);
                line->curs_pos = (old_b_pos < line->length) ? old_b_pos : line->length;
                break;
            }

            case '\n':
            {
                line_i++;

                short new_length = line->length - line->curs_pos;

                insert_line(line_i);
                append_string(textarea->lines[line_i], &line->buffer[line->curs_pos], new_length);

                line->length -= new_length;
                line->buffer[line->curs_pos] = '\0';

                wclear_from(mainwin, y, line->curs_pos + 1, new_length);

                x = 1;

                if (line_i > max_height && y == max_height + 1)
                {
                    textarea->linestart++;
                    print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                }
                else
                {
                    y++;
                    if (textarea->n_lines <= max_height + 1)
                    {
                        print_lines(y - 1, line_i - 1, textarea->n_lines - 1, max_width);
                    }
                    else
                    {
                        print_lines(y - 1, line_i - 1, textarea->linestart + max_height, max_width);
                    }
                }

                line = textarea->lines[line_i];
                break;
            }

            // Quit when pressing Ctrl + 'q'
            case CTRL('q'):
                destroy_textarea();
                return ACT_RETURN;

            default:
                if ((ch == KEY_BACKSPACE || ch == '\b' || ch == 127) && line->curs_pos == 0 && line_i > 0)
                {
                    line_i--;

                    LINE ** lines = textarea->lines;

                    x = ((lines[line_i]->length + 1) > max_width) ? max_width : (lines[line_i]->length + 1);
                    lines[line_i]->curs_pos = lines[line_i]->length;

                    append_string(lines[line_i], lines[line_i + 1]->buffer, lines[line_i + 1]->length);
                    delete_line(line_i + 1);

                    wclear_from(mainwin, y, 1, max_width);

                    if (textarea->n_lines > max_height && textarea->linestart > 0)
                    {
                        textarea->linestart--;
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    }
                    else
                    {
                        y--;
                        if (textarea->n_lines <= max_height + 1)
                        {
                            print_lines(y, line_i, textarea->n_lines - 1, max_width);
                            wclear_from(mainwin, textarea->n_lines + 1, 1, max_width);
                        }
                        else
                        {
                            print_lines(y, line_i, textarea->linestart + max_height, max_width);
                        }
                    }
                }
                else
                {
                    handle_line(mainwin, line, ch, &x, max_width, y);
                    if (line->length >= max_width)
                    {
                        textarea->strstart = line->strstart;
                        if (textarea->n_lines > max_height)
                            print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                        else
                            print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                        break;
                    }
                    edited = true;
                }
        }

        if (edited)
        {
            mvwprintw(mainwin, y, 1, "%.*s", max_width, &line->buffer[textarea->strstart]);
            edited = false;
        }
        wmove(mainwin, y, x);
        wrefresh(mainwin);
    }

    return ACT_CONTINUE;
}

// Allocate memory for and initialize the textarea
void init_textarea(void)
{
    textarea = (TEXTAREA *)malloc(sizeof(TEXTAREA));

    textarea->n_lines = 1;
    textarea->strstart = textarea->linestart = 0;
    init_lines(&textarea->lines, textarea->n_lines);
}

// Destroy and free memory used by the textarea
void destroy_textarea(void)
{
    for (int i = 0; i < textarea->n_lines; i++)
        free(textarea->lines[i]->buffer);
    free_arr((void **)textarea->lines, textarea->n_lines);
}

// Print an array of lines
static void print_lines(short starty, int from, int to, short max_width)
{
    for (int i = from; i <= to; i++, starty++)
    {
        LINE * line = textarea->lines[i];

        wclear_from(mainwin, starty, 1, max_width);
        if (line->length > textarea->strstart)
            mvwprintw(mainwin, starty, 1, "%.*s", max_width, &line->buffer[textarea->strstart]);
        wrefresh(mainwin);
    }
}

// Append a string to the end of a line
static void append_string(LINE * line, char * string, int length)
{
    if (line->length + length + 1 >= line->capacity)
    {
        line->capacity = GET_CAPACITY(line->length + length + 1);
        line->buffer = GROW_ARRAY(char, line->buffer, line->capacity);
    }

    memcpy(&line->buffer[line->length], string, length + 1);

    line->length += length;
}

// Insert a line
static void insert_line(int at)
{
    textarea->lines = GROW_ARRAY(LINE *, textarea->lines, ++textarea->n_lines);

    if (at + 1 < textarea->n_lines)
        memmove(&textarea->lines[at + 1], &textarea->lines[at], sizeof(LINE *) * (textarea->n_lines - at - 1));

    textarea->lines[at] = create_newline();
}

// Delete a line from memory
static void delete_line(int at)
{
    textarea->n_lines--;

    if (at < textarea->n_lines)
        memmove(&textarea->lines[at], &textarea->lines[at + 1], sizeof(LINE *) * (textarea->n_lines - at));

    textarea->lines = SHRINK_ARRAY(LINE *, textarea->lines, textarea->n_lines);
}
