#include <stdlib.h>
#include "form.h"

static FORM * new_form(int height, int width, int starty, int startx, int type);
static void init_fields(FIELD ***, short n_fields);
static void init_buttons(BUTTON ***, short n_buttons);
static void add_field(FORM *, FIELD *, const char * label, short height, short width, short ypos, bool hidden);
static void add_button(FORM *, BUTTON *, const char * content, short ypos, short xpos);

// Create a login form
FORM * create_loginform(void)
{
    // Center the window
    int starty = (LINES - form_height) / 2,
        startx = (COLS - form_width) / 2;

    // Init form
    FORM * form = new_form(form_height, form_width, starty, startx, 1);

    mvwprintw(form->win, 2, (form->cols - 5) / 2, "Login");
    wrefresh(form->win);

    // Allocate memory for two text fields
    form->n_fields = 2;
    init_fields(&form->fields, form->n_fields);
    // Allocate memory for two buttons
    form->n_buttons = 2;
    init_buttons(&form->buttons, form->n_buttons);

    short field_width = form_width - 12;
    short field_height = 3;
    short padding = (form_width - field_width) / 2;

    // First field (email)
    add_field(form, form->fields[0], " - Email Address :", field_height, field_width, 7, false);
    // Second field (password)
    add_field(form, form->fields[1], " - Password :", field_height, field_width, 12, true);

    // First button (submit)
    add_button(form, form->buttons[0], "  Submit  ", 17, padding + 2);
    // Second button (register)
    mvwprintw(form->win, 19, padding - 1, " Don't have an account?");
    wrefresh(form->win);
    add_button(form, form->buttons[1], "  Register  ", 21, padding + 2);

    return form;
}

// Create a registration form
FORM * create_registrform(void)
{
    // Center the window
    int starty = (LINES - form_height) / 2,
        startx = (COLS - form_width) / 2;

    // Init form
    FORM * form = new_form(form_height, form_width, starty, startx, 1);

    mvwprintw(form->win, 2, (form->cols - 8) / 2, "Register");
    wrefresh(form->win);

    // Allocate memory for three text fields
    form->n_fields = 3;
    init_fields(&form->fields, form->n_fields);
    // Allocate memory for two buttons
    form->n_buttons = 2;
    init_buttons(&form->buttons, form->n_buttons);

    short field_width = form_width - 12;
    short field_height = 3;
    short padding = (form_width - field_width) / 2;

    // First field (username)
    add_field(form, form->fields[0], " - Username :", field_height, field_width, 7, false);
    // First field (email)
    add_field(form, form->fields[1], " - Email Address :", field_height, field_width, 12, false);
    // Second field (password)
    add_field(form, form->fields[2], " - Password :", field_height, field_width, 17, true);

    // First button (submit)
    add_button(form, form->buttons[0], "  Submit  ", 20, padding + 2);
    // Second button (register)
    mvwprintw(form->win, 22, padding - 1, " Already have an account?");
    wrefresh(form->win);
    add_button(form, form->buttons[1], "  Login  ", 24, padding + 2);

    return form;
}

// Create an initial form
static FORM * new_form(int height, int width, int starty, int startx, int type)
{
    // Allocate memory for the form
    FORM * form = (FORM *)malloc(sizeof(FORM));
    // Init form window
    form->win = create_newwin(height, width, starty, startx, type);

    form->rows = height;
    form->cols = width;

    form->fields = NULL;
    form->active_f = NULL;
    form->buttons = NULL;
    form->active_b = NULL;

    return form;
}

// Initialize fields
static void init_fields(FIELD *** fields, short n_fields)
{
    *fields = (FIELD **)malloc(n_fields * sizeof(FIELD *));
    for (int i = 0; i < n_fields; i++)
    {
        (*fields)[i] = (FIELD *)malloc(sizeof(FIELD));
        (*fields)[i]->capacity = 8;
        (*fields)[i]->length = (*fields)[i]->cur_pos = 0;
        (*fields)[i]->buffer = (char *)malloc((*fields)[i]->capacity * sizeof(char));
        (*fields)[i]->buffer[0] = '\0';
    }
}

// Initialize buttons
static void init_buttons(BUTTON *** buttons, short n_buttons)
{
    *buttons = (BUTTON **)malloc(n_buttons * sizeof(BUTTON *));
    for (int i = 0; i < n_buttons; i++)
        (*buttons)[i] = (BUTTON *)malloc(sizeof(BUTTON));
}

static void add_field(FORM * form, FIELD * field, const char * label, short height, short width, short ypos, bool hidden)
{
    field->rows = height;
    field->cols = width;
    mvwprintw(form->win, ypos - 2, (form_width - width) / 2, label);
    wrefresh(form->win);
    field->win = create_newwin(height, width, ypos, (COLS - width) / 2, 0);
    field->hidden = hidden;
}

static void add_button(FORM * form, BUTTON * button, const char * content, short ypos, short xpos)
{
    button->xpos = xpos;
    button->ypos = ypos;

    button->content = content;
    button->style = A_BLINK;
    button->highlight = A_STANDOUT;

    wattron(form->win, button->style);
    mvwprintw(form->win, ypos, xpos, button->content);
    wattroff(form->win, button->style);

    wrefresh(form->win);
}

// Destroy a form
void destroy_form(FORM * form)
{
    destroy_win(form->fields[0]->win);
    destroy_win(form->fields[1]->win);
    destroy_win(form->win);
    free(form->fields);
    free(form);
}

// Highlight button
void highlight_b(FORM * form, short index)
{
    for (int i = 0; i < form->n_buttons; i++)
    {
        short xpos = form->buttons[i]->xpos;
        short ypos = form->buttons[i]->ypos;
        BUTTON * button = form->buttons[i];

        if (i == index)
        {
            wattron(form->win, button->highlight);
            mvwprintw(form->win, ypos, xpos, button->content);
            wattroff(form->win, button->highlight);
        }
        else
        {
            wattron(form->win, button->style);
            mvwprintw(form->win, ypos, xpos, button->content);
            wattroff(form->win, button->style);
        }

        wrefresh(form->win);
    }
}

// Remove highlight
void reset_buttons(FORM * form)
{
    highlight_b(form, -1);
}
