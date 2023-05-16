#include <string.h>
#include <math.h>
#include <dirent.h>
#include "notepad.h"
#include "globals.h"
#include "menu.h"

#define menu_height 21
#define menu_width  64

#define notes_height    24
#define notes_width     84

#define notes_startx    ((mainwin_width) - (notes_width)) / 2
#define notes_starty    6

bool notepad_called;

char * notes_dir;
char * note_dir;

TEXTAREA * textarea;
NOTELIST * note_list;
FILE * fnote;

static void save_note(void);
static void load_note(int);
static void save_notes(void);
static void load_notes(void);

static void init_notelist(void);
static void init_textarea(void);
void destroy_textarea(void);

static act_result editable_note(int);
static act_result uneditable_note(int);
static act_result new_editable_note(int);
static act_result display_note(int, bool editable, bool empty);
static act_result delete_note(int);

static act_result display_notes(event, chtype highlight);
static act_result display_all_notes(void);
static act_result new_note(void);
static act_result edit_note(void);
static act_result remove_note(void);

static void update_notes(BUTTON ***, event, chtype highlight, int * n_buttons);

bool file_exists(char * name);

static void print_lines(short starty, int from, int to, short max_width);
static void append_string(LINE *, char *, int length);
// Insert a line
static void insert_line(int at);
// Deallocate memory reseved for a line
static void delete_line(int at);

static void add_note_tolist(char * content);
static void refresh_notes_win(void);
static void refresh_notepad(bool editable, short max_height);

act_result notepad(void)
{
    notepad_called = true;

    notes_dir = create_newdir("notes", db_ext);

    init_notelist();
    load_notes();

    while (true)
    {
        dim_box box = {0};
        {
            BUTTON tmp = {0};

            box = (dim_box){
                .height = 3,
                .width = 32,
                .xpos = -1,
                .ypos = 2
            };

            add_button(mainwin, &tmp, "Notepad", box, COLOR_PAIR(WHITE_BG_BLACK), 0);
        }

        box = (dim_box){
            .height = menu_height,
            .width = menu_width,
            .xpos = sidebar_width + (mainwin_width - menu_width) / 2,
            .ypos = 8
        };
        MENU * menu = create_newmenu(box, 5);

        box = (dim_box){
            .height = 3,
            .width = menu_width - 12,
            .xpos = -1,
            .ypos = 1
        };
        add_button(menu->win, menu->buttons[0], "Display all notes", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[1], "Create a new note", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[2], "Edit a note", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[3], "Remove a note", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[4], "Quit", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(RED_BG_WHITE));

        menu->buttons[0]->action = &display_all_notes;
        menu->buttons[1]->action = &new_note;
        menu->buttons[2]->action = &edit_note;
        menu->buttons[3]->action = &remove_note;
        menu->buttons[4]->action = &__quit__;

        act_result result = handle_menu(menu);

        if (result == ACT_RETURN) return ACT_RETURN;
        if (result == ACT_CONTINUE) continue;

        wrefresh(mainwin);
    }

    return ACT_CONTINUE;
}

static void init_notelist(void)
{
    note_list = (NOTELIST *)calloc(1, sizeof(NOTELIST));
}

void destroy_notelist(void)
{
    if (!notepad_called) return;
    free_arr((void **)note_list->notes, note_list->n_notes);
    free(note_list);
    free(notes_dir);
}

static act_result display_notes(event e, chtype highlight)
{
    reset_mainwin();

    WINDOW * notes_win = create_newwin(notes_height, notes_width, notes_starty, sidebar_width + notes_startx, -1, 0);

    refresh_notes_win();

    if (note_list->n_notes > 0)
    {
        BUTTON ** note_buttons;
        update_notes(&note_buttons, e, highlight, &note_list->n_notes);

        handle_list(notes_win, &note_buttons, note_list->n_notes, notes_height / 4, &refresh_notes_win, &update_notes);

        free_arr((void **)note_buttons, note_list->n_notes);
    }

    if (note_list->n_notes == 0)
    {
        reset_mainwin();
        refresh_notes_win();

        mvwprintw(notes_win, 2, (notes_width - 17) / 2, "No notes available.");
        wrefresh(notes_win);
        while (true)
        {
            wchar_t ch = wgetch(notes_win);

            if (ch == CTRL('q')) break;
        }
    }

    reset_mainwin();
    destroy_win(notes_win);

    return ACT_CONTINUE;
}

static act_result display_all_notes(void)
{
    return display_notes(&uneditable_note, 0);
}

static act_result editable_note(int index)
{
    return display_note(index, true, false);
}

static act_result uneditable_note(int index)
{
    return display_note(index, false, false);
}

static act_result new_editable_note(int index)
{
    return display_note(index, true, true);
}

static act_result new_note(void)
{
    reset_mainwin();

    dim_box box = {
        .height = 3,
        .width = MAX_SIZE,
        .xpos = -1,
        .ypos = (mainwin_height - 7) / 2 + 2
    };
    box.xpos = sidebar_width + (mainwin_width - box.width) / 2 + 2;

    WINDOW * prompt = create_newwin(7, box.width, (mainwin_height - 7) / 2, sidebar_width + (mainwin_width - box.width) / 2, 1, COLOR_PAIR(CYAN));

    wattron(mainwin, COLOR_PAIR(GREEN));
    mvwprintw(mainwin, prompt->_begy + prompt->_maxy, prompt->_begx - sidebar_width + 2, "ENTER: Confirm");
    wattroff(mainwin, COLOR_PAIR(GREEN));
    mvwprintw(mainwin, prompt->_begy + prompt->_maxy + 1, prompt->_begx - sidebar_width + 2, "ARROWS: Navigate");
    mvwprintw(mainwin, prompt->_begy + prompt->_maxy + 2, prompt->_begx - sidebar_width + 2, "CTRL + Q: Quit/Cancel");
    wrefresh(mainwin);

    FIELD * text_field = create_newfield();
    box.width -= 4;
    add_field(mainwin, text_field, "Enter the name of the note file :", box, false, COLOR_PAIR(CYAN));

    short x = 1;
    short max_size = box.width - 3;

    // Show the cursor if hidden, then move it to the beginning of the first field
    curs_set(1);
    wmove(text_field->win, 1, 1);
    wrefresh(text_field->win);

    wchar_t ch;
    LINE * line = text_field->line;

    while (true)
    {
        ch = wgetch(text_field->win);

        switch (ch)
        {
            case KEY_LEFT: // Left key
                // Order of these 'if' statements matter A LOT
                if (line->length > max_size - 1 && x == 1 && line->strstart > 0)
                    line->strstart--;
                if (x > 1)
                    x--;
                if (line->curs_pos > 0)
                    line->curs_pos--;
                break;
            case KEY_RIGHT: // Right key
                // Order...
                if (line->length > max_size - 1 && x == max_size + 1 && line->curs_pos < line->length)
                    line->strstart++;
                if (x < line->length + 1 && x <= max_size)
                    x++;
                if (line->curs_pos < line->length)
                    line->curs_pos++;
                break;

            case '\n':
                if (line->length == 0)
                {
                    text_field->error = "File name cannot be empty.";
                    fld_printerr(text_field);
                }
                else if (!is_valid_filename(text_field->line->buffer, text_field->line->length))
                {
                    text_field->error = "Invalid file name.";
                    fld_printerr(text_field);
                }
                else if (file_exists(text_field->line->buffer))
                {
                    text_field->error = "File already exists.";
                    fld_printerr(text_field);
                }
                else
                    goto jump;
                break;

            case CTRL('q'):
                goto quit;

            // Ignore the following keys
            case KEY_IC: // Insert key
            case KEY_DC: // Delete key
            case KEY_HOME: // Home key
            case KEY_END: // End key
            case KEY_NPAGE: // Page down key
            case KEY_PPAGE: // Page up key
                break;

            default:
                handle_line(text_field->win, line, ch, &x, max_size, 1);
        }

        mvwprintw(text_field->win, 1, 1, "%.*s", max_size, &line->buffer[line->strstart]);

        wmove(text_field->win, 1, x);
        wrefresh(text_field->win);
    }

jump:
    line->strstart = line->curs_pos = 0;

    fld_clrerr(text_field);

    note_dir = create_newdir(text_field->line->buffer, note_ext);

    add_note_tolist(text_field->line->buffer);
    save_notes();

    box = (dim_box){
        .width = MAX_SIZE,
        .xpos = -1,
        .ypos = (mainwin_height - 7) / 2
    };
    box.xpos = sidebar_width + (mainwin_width - box.width) / 2;

    reset_mainwin();
    ALERT * alert = create_newalert(box, "Note added successfully!", COLOR_PAIR(GREEN_BG_BLACK));

    handle_alert(alert);

    destroy_field(text_field);
    destroy_win(prompt);

    reset_mainwin();
    curs_set(0);

    act_result result = new_editable_note(note_list->n_notes - 1);

    return result;

quit:
    destroy_field(text_field);
    destroy_win(prompt);

    reset_mainwin();
    curs_set(0);

    return ACT_CONTINUE;
}

static act_result delete_note(int index)
{
    reset_mainwin();

    dim_box box = {
        .width = 80,
        .xpos = sidebar_width + (mainwin_width - 80) / 2,
        .ypos = (mainwin_height - 7) / 2
    };

    PROMPT * prompt = create_newprompt(box, "Are you sure you want to remove this item?");

    bool cancel = !handle_prompt(prompt);

    if (cancel)
        return ACT_CONTINUE;
    else
    {
        note_list->n_notes--;

        char * tmp_dir = create_newdir(note_list->notes[index]->buffer, note_ext);//FREE
        remove(tmp_dir);

        destroy_line(note_list->notes[index]);
        if (index < note_list->n_notes)
            memmove(&note_list->notes[index], &note_list->notes[index + 1], sizeof(LINE *) * (note_list->n_notes - index));

        if (note_list->n_notes >= 1)
            note_list->notes = SHRINK_ARRAY(LINE *, note_list->notes, note_list->n_notes);

        save_notes();

        box = (dim_box){
            .width = MAX_SIZE,
            .xpos = -1,
            .ypos = (mainwin_height - 7) / 2
        };
        box.xpos = sidebar_width + (mainwin_width - box.width) / 2;

        reset_mainwin();
        ALERT * alert = create_newalert(box, "Note removed successfully!", COLOR_PAIR(GREEN_BG_BLACK));

        handle_alert(alert);

        return ACT_UPDATE;
    }
}

static act_result edit_note(void)
{
    return display_notes(&editable_note, 0);
}

static act_result remove_note(void)
{
    return display_notes(&delete_note, COLOR_PAIR(RED_BG_WHITE));
}

static void update_notes(BUTTON *** buttons, event e, chtype highlight, int * n_buttons)
{
    if (n_buttons != NULL)
        *n_buttons = note_list->n_notes;
    if (note_list->n_notes == 0)
        return;

    init_buttons(buttons, note_list->n_notes);

    dim_box box = {
        .height = 3,
        .width = notes_width,
        .xpos = -1,
        .ypos = 0
    };

    for (int i = 0; i < *n_buttons; i++)
    {
        LINE * fname = note_list->notes[i];

        char * tmp = (char *)malloc((notes_width + 1) * sizeof(char));
        sprintf(tmp, "%6d %5.2d/%.2d/%.4d", i + 1, 0, 0, 0);
        short curr_len = 5 + 1 + 4 + 10;

        #define OFFSET  4
        #define CUT     10

        if (curr_len + OFFSET + fname->length <= notes_width - CUT)
        {
            append_offset(tmp, fname->buffer, OFFSET, curr_len, fname->length);
            append_spaces(tmp, notes_width - (curr_len + OFFSET + fname->length), curr_len + OFFSET + fname->length);
        }
        else
        {
            append_offset(tmp, fname->buffer, OFFSET, curr_len, notes_width - CUT - (curr_len + OFFSET));
            strcat(tmp, "...");
            append_spaces(tmp, CUT - 3, notes_width - (CUT - 3));
        }

        #undef OFFSET
        #undef CUT

        create_newbutton((*buttons)[i], tmp, box, 0, highlight);
        (*buttons)[i]->action = e;
    }
}

// Allocate memory for and initialize the textarea
void init_textarea(void)
{
    textarea = (TEXTAREA *)malloc(sizeof(TEXTAREA));

    textarea->n_lines = 1;
    textarea->linestart = 0;
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
        mvwprintw(mainwin, starty, 1, "%.*s", max_width, line->buffer);
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
    if (textarea->n_lines == 0)
    {
        init_lines(&textarea->lines, 1);
        textarea->n_lines = 1;
        return;
    }

    textarea->lines = GROW_ARRAY(LINE *, textarea->lines, ++textarea->n_lines);

    if (at + 1 < textarea->n_lines)
        memmove(&textarea->lines[at + 1], &textarea->lines[at], sizeof(LINE *) * (textarea->n_lines - at - 1));

    textarea->lines[at] = create_newline();
}

// Delete a line from memory
static void delete_line(int at)
{
    textarea->n_lines--;

    destroy_line(textarea->lines[at]);
    if (at < textarea->n_lines)
        memmove(&textarea->lines[at], &textarea->lines[at + 1], sizeof(LINE *) * (textarea->n_lines - at));

    textarea->lines = SHRINK_ARRAY(LINE *, textarea->lines, textarea->n_lines);
}

static act_result display_note(int index, bool editable, bool empty)
{
    reset_mainwin();

    int line_i = 0; // Line index
    short x = 1; // Cursor x position relative to `mainwin`
    short y = 1; // Cursor y position relative to `mainwin`
    const short max_width = mainwin_width - 3;
    const short max_height = mainwin_height - 10;
    bool edited = false;

    refresh_notepad(editable, max_height);

    if (!empty)
    {
        textarea = (TEXTAREA *)calloc(1, sizeof(TEXTAREA));
        load_note(index);
        if (textarea->n_lines <= max_height + 1)
        {
            print_lines(1, 0, textarea->n_lines - 1, max_width);
        }
        else
        {
            print_lines(1, 0, max_height, max_width);
        }
    }
    else
    {
        init_textarea();
        save_note();
    }

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
                /*if (line->length >= max_width && x == 1 && textarea->strstart > 0)
                {
                    textarea->strstart--;
                    line->strstart--;
                    if (textarea->n_lines > max_height)
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    else
                        print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                }*/
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
                /*if (line->length >= max_width && x == max_width + 1 && line->curs_pos < line->length)
                {
                    textarea->strstart++;
                    line->strstart++;
                    if (textarea->n_lines > max_height)
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    else
                        print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                }*/
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

                /*if (textarea->strstart > 0 && line_i > 0 && textarea->lines[line_i - 1]->length <= old_b_pos)
                {
                    line = textarea->lines[line_i - 1];
                    if (textarea->strstart >= line->length)
                    {
                        textarea->strstart = clamp(textarea->strstart - max_width, 0, max_width);
                        x = max_width + 1;
                    }
                    else
                    {
                        x = line->length - textarea->strstart + 1;
                        textarea->lines[line_i - 1]->strstart = textarea->strstart;
                    }

                    if (textarea->n_lines > max_height)
                        print_lines(1, textarea->linestart, textarea->linestart + max_height, max_width);
                    else
                        print_lines(1, textarea->linestart, textarea->n_lines - 1, max_width);
                }*/
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
                if (!editable) break;

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
                reset_mainwin();
                return ACT_CONTINUE;
            case CTRL('s'):
                if (editable)
                {
                    save_note();

                    dim_box box = (dim_box){
                        .width = MAX_SIZE,
                        .xpos = -1,
                        .ypos = (mainwin_height - 7) / 2
                    };
                    box.xpos = sidebar_width + (mainwin_width - box.width) / 2;

                    reset_mainwin();
                    ALERT * alert = create_newalert(box, "File saved successfully!", COLOR_PAIR(GREEN_BG_BLACK));
                    handle_alert(alert);

                    if (textarea->n_lines <= max_height + 1)
                    {
                        print_lines(1, 0, textarea->n_lines - 1, max_width);
                    }
                    else
                    {
                        print_lines(1, 0, max_height, max_width);
                    }

                    refresh_notepad(true, max_height);
                    curs_set(1);
                }
                break;

            // Ignore the following keys
            case KEY_IC: // Insert key
            case KEY_DC: // Delete key
            case KEY_HOME: // Home key
            case KEY_END: // End key
            case KEY_NPAGE: // Page down key
            case KEY_PPAGE: // Page up key
                break;

            default:
                if (!editable) break;
                if (IS_BACKSPACE(ch) && line->curs_pos == 0 && line_i > 0)
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
                else if (line->length < max_width || IS_BACKSPACE(ch))
                {
                    handle_line(mainwin, line, ch, &x, max_width, y);
                    if (line->length >= max_width)
                    {
                        /*textarea->strstart = line->strstart;*/
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
            mvwprintw(mainwin, y, 1, "%.*s", max_width, line->buffer);
            edited = false;
        }
        wmove(mainwin, y, x);
        wrefresh(mainwin);
    }

    return ACT_CONTINUE;
}

static void refresh_notes_win(void)
{
    mvwprintw(mainwin, 4, notes_startx + 5, "ID");
    mvwprintw(mainwin, 4, notes_startx + 12, "DATE");
    mvwprintw(mainwin, 4, notes_startx + 24, "FILE NAME");

    for (int i = 0; i < notes_width; i++)
        mvwaddch(mainwin, 5, notes_startx + i, H_LINE);

    mvwprintw(mainwin, notes_starty + notes_height + 1, notes_startx + 2, "ARROWS: Navigate");
    mvwprintw(mainwin, notes_starty + notes_height + 2, notes_startx + 2, "ENTER: Select");
    mvwprintw(mainwin, notes_starty + notes_height + 3, notes_startx + 2, "CTRL + Q: Quit");

    wrefresh(mainwin);
}

static void save_note(void)
{
    fnote = fopen(note_dir, "w");
    fassert(fnote, note_dir);

    for (int i = 0; i < textarea->n_lines; i++)
        fprintf(fnote, "%s\n", textarea->lines[i]->buffer);

    fclose(fnote);
}

static void load_note(int index)
{
    note_dir = create_newdir(note_list->notes[index]->buffer, note_ext);
    fnote = fopen(note_dir, "r");
    fassert(fnote, note_dir);

    fseek(fnote, 0, SEEK_END);
    size_t file_size = ftell(fnote);
    fseek(fnote, 0, SEEK_SET);
    size_t cur_pos = ftell(fnote);

    if (file_size == 0)
    {
        insert_line(0);
        goto quit;
    }

    while (cur_pos != file_size)
    {
        char * line = dfscans(fnote);

        insert_line(textarea->n_lines);
        append_string(textarea->lines[textarea->n_lines - 1], line, strlen(line));

        cur_pos = ftell(fnote);
    }

quit:
    fclose(fnote);
}

static void save_notes(void)
{
    FILE * fnotes = fopen(notes_dir, "w");
    fassert(fnotes, notes_dir);

    for (int i = 0; i < note_list->n_notes; i++)
        fprintf(fnotes, "%s\n", note_list->notes[i]->buffer);

    fclose(fnotes);
}

void load_notes(void)
{
    if (access(notes_dir, R_OK) != 0)
        return;

    FILE * fnotes = fopen(notes_dir, "r");
    fassert(fnotes, notes_dir);

    while (!feof(fnotes))
    {
        char * string = dfscans(fnotes);
        if (!(*string))
            break;

        add_note_tolist(string);
    }

    fclose(fnotes);
}

bool file_exists(char * name)
{
    char * notes_db_dir = create_newdir("notes", db_ext);
    FILE * notes_db = fopen(notes_db_dir, "r");

    if (!notes_db)
        return false;

    for (int i = 0; i < note_list->n_notes; i++)
    {
        char * tmp = dfscans(notes_db);
        if (!tmp[0] || strcmp(tmp, name) == 0)
            return true;
    }

    fclose(notes_db);

    return false;
}

static void add_note_tolist(char * content)
{
    int length = strlen(content),
        capacity = GET_CAPACITY(length);

    note_list->n_notes++;
    note_list->notes = GROW_ARRAY(LINE *, note_list->notes, note_list->n_notes);

    LINE * note = note_list->notes[note_list->n_notes - 1] = create_newline();

    note->length = length;
    note->capacity = capacity;

    note->buffer = (char *)malloc((capacity) * sizeof(char));

    strncpy(note->buffer, content, length + 1);
}

static void refresh_notepad(bool editable, short max_height)
{
    if (editable)
    {
        mvwprintw(mainwin, max_height + 3, 2, "- Don't forget to save after editing the file.");
        wattron(mainwin, COLOR_PAIR(GREEN));
        mvwprintw(mainwin, max_height + 4, 2, "CTRL + S: Save");
        wattroff(mainwin, COLOR_PAIR(GREEN));
    }
    mvwprintw(mainwin, max_height + 5, 2, "ARROWS: Navigate");
    mvwprintw(mainwin, max_height + 6, 2, "CTRL + Q: Quit/Cancel");

    wattron(mainwin, COLOR_PAIR(CYAN));
    for (int i = 1; i < mainwin_width - 1; i++)
        mvwaddch(mainwin, max_height + 2, i, H_LINE);
    wattroff(mainwin, COLOR_PAIR(CYAN));

    wrefresh(mainwin);
}
