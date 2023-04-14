#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include "login.h"

static FORM * g_form;

void get_user_input(void);
int is_valid_char(wchar_t);

static char * hide_str(int length);

// Initiate the login interface
void init_login(void)
{
    // Initialize and set default form
    refresh();
    g_form = create_loginform();

    get_user_input();
}

// Get the user input and refresh the window accordingly
void get_user_input(void)
{
    wchar_t ch; // Character to scan
    short x = 1; // Position of the cursor relative to the field
    short i = 0; // Index of the selected field/button
    int b_pos = 0, // Position of the cursor relative to the buffer
        strstart = 0; // Index of the beginning of the string to print
    bool in_fields = true; // A flag to check if a field is selected
    bool is_login = true; // A flag to check if the user is trying to login or register
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
            reset_buttons(g_form); \
            i = n - 1; \
            in_fields = boolval; \
        }

    #define DOWN_CHECK(n, boolval) \
        if (i < n - 1) \
            i++; \
        else if (i == n - 1) \
        { \
            reset_buttons(g_form); \
            i = 0; \
            in_fields = boolval; \
        }

    // Keep scanning characters (and keys) from the user until a new line is encountered
    while (true)
    {
        ch = wgetch(g_form->fields[i]->win);

        if (ch == '\n')
        {
            // If the user hits Enter while typing
            //      or if they hit Enter while highlighting the Submit button
            if (in_fields || (!in_fields && i == 0)); // Submit data
            else
            {
                // Destroy the current form
                destroy_form(g_form);
                // Create a new one accordingly
                g_form = is_login ? create_registrform() : create_loginform();
                // Highlight the first text field and set the cursor to the beginning
                in_fields = true; i = 0; x = 1; b_pos = 0;
                curs_set(1);
                wmove(g_form->fields[0]->win, 1, 1);
                wrefresh(g_form->fields[0]->win);
                // Switch between login and register page
                is_login = !is_login;

                continue;
            }
        }

        FIELD * field = g_form->fields[i];
        switch (ch)
        {
            case KEY_LEFT: // Left key
                if (in_fields)
                {
                    // Order of these 'if' statements matter A LOT
                    //      I learned that the hard way... *sigh*
                    if (field->length > max_size - 1 && x == 1 && strstart > 0)
                        strstart--;
                    if (x > 1)
                        x--;
                    if (b_pos > 0)
                        b_pos--;
                }
                break;
            case KEY_RIGHT: // Right key
                if (in_fields)
                {
                    // Order...
                    if (field->length > max_size - 1 && x == max_size + 1 && b_pos < field->length)
                        strstart++;
                    if (x < field->length + 1 && x <= max_size)
                        x++;
                    if (b_pos < field->length)
                        b_pos++;
                }
                break;
            case KEY_UP: // Up key
                if (in_fields)
                {
                    UP_CHECK(g_form->n_buttons, false);
                }
                else
                {
                    UP_CHECK(g_form->n_fields, true);
                }
                field = g_form->fields[i];
                x = ((field->length + 1) > max_size) ? (max_size + 1) : (field->length + 1);
                b_pos = field->length;
                strstart = ((field->length + 1) > (max_size)) ? (field->length - max_size) : 0;
                break;
            case KEY_DOWN: case '\t': // Down key or Tab key
                if (in_fields)
                {
                    DOWN_CHECK(g_form->n_fields, false);
                }
                else
                {
                    DOWN_CHECK(g_form->n_buttons, true);
                }
                field = g_form->fields[i];
                x = ((field->length + 1) > max_size) ? (max_size + 1) : (field->length + 1);
                b_pos = field->length;
                strstart = (field->length > (max_size + 1)) ? (field->length - max_size) : 0;
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
                {
                    if (isprint(ch))
                    {
                        // Allocate more memory if necessary
                        if (field->length + 1 >= field->capacity)
                        {
                            field->buffer = GROW_ARRAY(char, field->buffer, field->capacity);
                            field->capacity = GROW_CAPACITY(field->capacity);
                        }

                        if (field->length++ == 0) // If the field is empty
                        {
                            // Simply add the character to the beginning
                            field->buffer[0] = ch;
                            field->buffer[1] = '\0';
                        }
                        else
                        {
                            // Move the string from position (b_pos) one character to the right
                            //      to make room for the new character
                            memmove(&field->buffer[b_pos + 1], &field->buffer[b_pos], field->length - b_pos + 1);
                            // Insert the new character
                            field->buffer[b_pos] = ch;
                        }

                        // Order...
                        if (field->length > max_size && x == max_size + 1)
                            strstart++;
                        if (x <= max_size)
                            x++;

                        b_pos++;
                    }
                    // If the user hits a backspace
                    else if ((ch == KEY_BACKSPACE || ch == '\b' || ch == 127) && field->length > 0 && b_pos > 0)
                    {
                        if (field->length > max_size && strstart > 0)
                        {
                            // Move the whole string to the right
                            strstart--;
                            // Remove last (duplicate) character from the window
                            mvwaddch(field->win, 1, max_size, ' ');
                        }
                        else
                        {
                            // Move the cursor back one character
                            x--;
                            // Remove last (duplicate) character from the window
                            if (field->length <= max_size)
                                mvwaddch(field->win, 1, field->length, ' ');
                        }

                        // Move the string from position (x) one character to the left
                        memmove(&field->buffer[b_pos - 1], &field->buffer[b_pos], field->length - b_pos + 1);

                        field->length--;
                        b_pos--;
                    }
                }
        }

        if (in_fields)
        {
            // Print the new string
            mvwprintw(field->win, 1, 1, "%.*s", max_size, (field->hidden ? hide_str(field->length) : &field->buffer[strstart]));
            // Show cursor, then move it to the corresponding input field
            curs_set(1);
            wmove(field->win, 1, x);
            wrefresh(field->win);
        }
        else
        {
            // Hide cursor and highlight the corresponding button
            curs_set(0);
            highlight_b(g_form, i);
        }
    }

    #undef DOWN_CHECK
    #undef UP_CHECK
}

static char * hide_str(int length)
{
    char * hidden = (char *)malloc((length + 1) * sizeof(char));

    for (int i = 0; i <= length; i++)
        hidden[i] = '*';
    hidden[length] = '\0';

    return hidden;
}
