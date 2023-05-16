#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"
#include "form.h"
#include "login.h"
#include "loginhandler.h"

#define title_ypos 3
#define first_button_ypos 18

FORM * create_loginform(void);
FORM * create_registrform(void);

static act_result get_user_input(void);
int is_valid_char(wchar_t);

static char * hide_str(int length);

static act_result submit(void);
static act_result switch_forms(void);

static FORM * g_form;

bool is_login; // A flag to check if the user is trying to login or register

// Initiate the login interface
act_result init_login(void)
{
    // Initialize and set default form
    refresh();
    g_form = create_loginform();
    is_login = true;

    act_result res = get_user_input();

    destroy_form(g_form);
    return res;
}

// Get the user input and refresh the window accordingly
static act_result get_user_input(void)
{
    wchar_t ch; // Character to scan
    short x = 1; // Position of the cursor relative to the field
    short i = 0; // Index of the selected field/button
    bool in_fields = true; // A flag to check if a field is selected
    bool hide_pw = true;
    const short max_size = g_form->fields[i]->cols - 3; // Max size of the string to print

    // Show the cursor if hidden, then move it to the beginning of the first field
    curs_set(1);
    wmove(g_form->fields[0]->win, 1, 1);
    wrefresh(g_form->fields[0]->win);

    // Utility macros
    #define UP_CHECK(n, boolval) \
        if (i > 0) \
            i--; \
        else \
        { \
            i = n - 1; \
            in_fields = boolval; \
        }

    #define DOWN_CHECK(n, boolval) \
        if (i < n - 1) \
            i++; \
        else if (i == n - 1) \
        { \
            i = 0; \
            in_fields = boolval; \
        }

    #define highlight_b(i) \
        change_button_style(g_form->buttons, g_form->win, g_form->n_buttons, i, g_form->buttons[i]->highlight)
    #define reset_b(i) \
        change_button_style(g_form->buttons, g_form->win, g_form->n_buttons, i, g_form->buttons[i]->style)

    // Keep scanning characters (and keys) from the user until a new line is encountered
    while (true)
    {
        if (in_fields)
            ch = wgetch(g_form->fields[i]->win);
        else
            ch = wgetch(g_form->win);

        if (ch == '\n')
        {
            // If the user hits Enter while typing
            //      or if they hit Enter while highlighting the Submit button
            act_result result;
            if (in_fields)
                result = g_form->buttons[0]->action();
            else
                result = g_form->buttons[i]->action();

            if (result == ACT_CONTINUE)
            {
                // Highlight the first text field and set the cursor to the beginning
                if (!in_fields) reset_b(i);
                in_fields = true;
                i = 0;
                x = 1;
                g_form->fields[0]->line->curs_pos = 0;
                g_form->fields[0]->line->strstart = 0;
                curs_set(1);
                wmove(g_form->fields[0]->win, 1, 1);
                wrefresh(g_form->fields[0]->win);

                continue;
            }
            else return result;
        }

        FIELD * field;
        LINE * line;

        if (in_fields)
        {
            field = g_form->fields[i];
            line = field->line;
        }
        switch (ch)
        {
            case KEY_LEFT: // Left key
                if (in_fields)
                {
                    // Order of these 'if' statements matter A LOT
                    //      I learned that the hard way... *sigh*
                    if (line->length > max_size - 1 && x == 1 && line->strstart > 0)
                        line->strstart--;
                    if (x > 1)
                        x--;
                    if (line->curs_pos > 0)
                        line->curs_pos--;
                }
                break;
            case KEY_RIGHT: // Right key
                if (in_fields)
                {
                    // Order...
                    if (line->length > max_size - 1 && x == max_size + 1 && line->curs_pos < line->length)
                        line->strstart++;
                    if (x < line->length + 1 && x <= max_size)
                        x++;
                    if (line->curs_pos < line->length)
                        line->curs_pos++;
                }
                break;
            case KEY_UP: // Up key
                if (in_fields)
                {
                    UP_CHECK(g_form->n_buttons, false);
                }
                else
                {
                    reset_b(i);
                    UP_CHECK(g_form->n_fields, true);
                }
                if (0 <= i && i < g_form->n_fields)
                {
                    field = g_form->fields[i];
                    line = field->line;
                    x = ((line->length + 1) > max_size) ? (max_size + 1) : (line->length + 1);
                    line->curs_pos = line->length;
                    line->strstart = ((line->length + 1) > (max_size)) ? (line->length - max_size) : 0;
                }
                break;
            case KEY_DOWN: case '\t': // Down key or Tab key
                if (in_fields)
                {
                    DOWN_CHECK(g_form->n_fields, false);
                }
                else
                {
                    reset_b(i);
                    DOWN_CHECK(g_form->n_buttons, true);
                }
                if (0 <= i && i < g_form->n_fields)
                {
                    field = g_form->fields[i];
                    line = field->line;
                    x = ((line->length + 1) > max_size) ? (max_size + 1) : (line->length + 1);
                    line->curs_pos = line->length;
                    line->strstart = (line->length > (max_size + 1)) ? (line->length - max_size) : 0;
                }
                break;
            case CTRL('x'):
                hide_pw = !hide_pw;
                i = g_form->n_fields - 1;
                field = g_form->fields[i];
                line = field->line;
                x = ((line->length + 1) > max_size) ? (max_size + 1) : (line->length + 1);
                line->curs_pos = line->length;
                line->strstart = (line->length > (max_size + 1)) ? (line->length - max_size) : 0;
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
                if (in_fields)
                    handle_line(field->win, line, ch, &x, max_size, 1);
        }

        if (in_fields)
        {
            // Print the new string
            if (field->hidden && hide_pw)
                mvwprintw(field->win, 1, 1, "%.*s", max_size, hide_str(line->length));
            else
                mvwprintw(field->win, 1, 1, "%.*s", max_size, &line->buffer[line->strstart]);
            // Show cursor, then move it to the corresponding input field
            curs_set(1);
            wmove(field->win, 1, x);
            wrefresh(field->win);
        }
        else
        {
            // Hide cursor and highlight the corresponding button
            curs_set(0);
            highlight_b(i);
        }
    }

    #undef DOWN_CHECK
    #undef UP_CHECK
    #undef highlight_b
    #undef reset_b
}

static char * hide_str(int length)
{
    char * hidden = (char *)malloc((length + 1) * sizeof(char));

    for (int i = 0; i < length; i++)
        hidden[i] = '*';
    hidden[length] = '\0';

    return hidden;
}

// Submit form
act_result submit(void)
{
    reseterr(g_form);
    clrerr(g_form);

    bool is_valid = validate_form(g_form, is_login);
    if (!is_valid)
    {
        printerr(g_form);
        return ACT_CONTINUE;
    }

    if (is_login)
        is_valid = login_user(g_form);
    else
        is_valid = register_user(g_form);

    if (!is_valid)
    {
        printerr(g_form);
        return ACT_CONTINUE;
    }

    return ACT_SUBMIT;
}

// Switch from between login and registration forms
act_result switch_forms(void)
{
    // Destroy the current form
    destroy_form(g_form);
    // Create a new one accordingly
    g_form = is_login ? create_registrform() : create_loginform();
    // Switch between login and register page
    is_login = !is_login;

    return ACT_CONTINUE;
}

// Create a login form
FORM * create_loginform(void)
{
    // Temporary info holder
    dim_box box = {0};
    // Center the window
    int starty = (LINES - form_height) / 2,
        startx = (COLS - form_width) / 2;

    // Init form
    FORM * form = new_form(form_height, form_width, starty, startx, 1, COLOR_PAIR(CYAN));

    mvprintw(starty + form_height + 1, startx + 1, "ARROWS: Navigate");
    mvprintw(starty + form_height + 2, startx + 1, "ENTER: Select/Confirm");
    mvprintw(starty + form_height + 2, startx + 1, "CTRL + X: Hide/Show password");
    refresh();

    mvwprintw(form->win, title_ypos, (form->cols - 5) / 2, "Login");

    // Allocate memory for two text fields
    form->n_fields = 2;
    init_fields(&form->fields, form->n_fields);
    // Allocate memory for two buttons
    form->n_buttons = 3;
    init_buttons(&form->buttons, form->n_buttons);

    short field_width = form_width - 12;
    short field_height = 3;
    short padding = (form_width - field_width) / 2;

    // First field (email)
    box = (dim_box){
        .height = field_height,
        .width = field_width,
        .xpos = (COLS - field_width) / 2,
        .ypos = title_ypos + starty + 2
    };
    add_field(form->win, form->fields[0], "Email Address", box, false, COLOR_PAIR(CYAN));
    // Second field (password)
    box.ypos += 4;
    add_field(form->win, form->fields[1], "Password", box, true, COLOR_PAIR(CYAN));

    // First button (submit)
    box = (dim_box){
        .height = 1,
        .width = 12,
        .xpos = padding + 2,
        .ypos = first_button_ypos
    };
    add_button(form->win, form->buttons[0], "Submit", box, COLOR_PAIR(CYAN_BG_BLACK), 0);
    form->buttons[0]->action = &submit;
    // Second button (register)
    mvwprintw(form->win, first_button_ypos + 2, padding - 1, " Don't have an account?");
    box.ypos += 4;
    add_button(form->win, form->buttons[1], "Register", box, COLOR_PAIR(CYAN_BG_BLACK), 0);
    form->buttons[1]->action = &switch_forms;

    box = (dim_box){
        .height = 3,
        .width = 5,
        .xpos = form_width - 2 - 5,
        .ypos = 1
    };
    add_button(form->win, form->buttons[2], "x", box, COLOR_PAIR(RED_BG_WHITE), 0);
    form->buttons[2]->action = &__quit__;

    wrefresh(form->win);

    return form;
}

// Create a registration form
FORM * create_registrform(void)
{
    // Temporary info holder
    dim_box box = {0};
    // Center the window
    int starty = (LINES - form_height) / 2,
        startx = (COLS - form_width) / 2;

    // Init form
    FORM * form = new_form(form_height, form_width, starty, startx, 1, COLOR_PAIR(CYAN));

    mvprintw(starty + form_height + 1, startx + 1, "ARROWS: Navigate");
    mvprintw(starty + form_height + 2, startx + 1, "ENTER: Select/Confirm");
    mvprintw(starty + form_height + 2, startx + 1, "CTRL + X: Hide/Show password");
    refresh();

    mvwprintw(form->win, title_ypos, (form->cols - 8) / 2, "Register");

    // Allocate memory for three text fields
    form->n_fields = 3;
    init_fields(&form->fields, form->n_fields);
    // Allocate memory for two buttons
    form->n_buttons = 3;
    init_buttons(&form->buttons, form->n_buttons);

    short field_width = form_width - 12;
    short field_height = 3;
    short padding = (form_width - field_width) / 2;

    // First field (username)
    box = (dim_box){
        .height = field_height,
        .width = field_width,
        .xpos = (COLS - field_width) / 2,
        .ypos = title_ypos + starty + 2
    };
    add_field(form->win, form->fields[0], "Username", box, false, COLOR_PAIR(CYAN));
    // First field (email)
    box.ypos += 4;
    add_field(form->win, form->fields[1], "Email Address", box, false, COLOR_PAIR(CYAN));
    // Second field (password)
    box.ypos += 4;
    add_field(form->win, form->fields[2], "Password", box, true, COLOR_PAIR(CYAN));

    // First button (submit)
    box = (dim_box){
        .height = 1,
        .width = 12,
        .xpos = padding + 2,
        .ypos = first_button_ypos
    };
    add_button(form->win, form->buttons[0], "Submit", box, COLOR_PAIR(CYAN_BG_BLACK), 0);
    form->buttons[0]->action = &submit;
    // Second button (register)
    mvwprintw(form->win, first_button_ypos + 2, padding - 1, " Already have an account?");
    box.ypos += 4;
    add_button(form->win, form->buttons[1], "Login", box, COLOR_PAIR(CYAN_BG_BLACK), 0);
    form->buttons[1]->action = &switch_forms;

    box = (dim_box){
        .height = 3,
        .width = 5,
        .xpos = form_width - 2 - 5,
        .ypos = 1
    };
    add_button(form->win, form->buttons[2], "x", box, COLOR_PAIR(RED_BG_WHITE), 0);
    form->buttons[2]->action = &__quit__;

    wrefresh(form->win);

    return form;
}
