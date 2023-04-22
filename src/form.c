#include <stdlib.h>
#include <string.h>
#include "form.h"

static void free_arr(void **, int n_elem);

// Create an initial form
FORM * new_form(int height, int width, int starty, int startx, int type, chtype colors)
{
    // Allocate memory for the form
    FORM * form = (FORM *)malloc(sizeof(FORM));
    // Init form window
    form->win = create_newwin(height, width, starty, startx, type, colors);

    form->rows = height;
    form->cols = width;

    form->fields = NULL;
    form->buttons = NULL;

    return form;
}

// Destroy a form
void destroy_form(FORM * form)
{
    for (int i = 0; i < form->n_fields; i++)
    {
        FIELD * field = form->fields[i];
        // Destroy the field window and deallocate designated memory
        destroy_win(field->win);
        // Free memory allocated for the field buffer
        free(field->line->buffer);
    }
    // Destroy the form window and deallocate designated memory
    destroy_win(form->win);
    // Free memory allocated for fields
    free_arr((void **)form->fields, form->n_fields);
    // Free memory allocated for buttons
    free_arr((void **)form->buttons, form->n_buttons);
    // Free memory allocated for the form
    free(form);
}

// Print errors
void printerr(FORM * form)
{
    attron(COLOR_PAIR(RED));
    for (int i = 0; i < form->n_fields; i++)
    {
        FIELD * field = form->fields[i];

        move(field->ypos - 1, field->xpos);
        if (field->error[0])
            printw(field->error);
        refresh();
    }
    attroff(COLOR_PAIR(RED));
}

// Clear errors
void clrerr(FORM * form)
{
    for (int i = 0; i < form->n_fields; i++)
    {
        FIELD * field = form->fields[i];
        clear_from(field->ypos - 1, field->xpos, field->cols);
        refresh();
    }
}

// Reset error messages
void reseterr(FORM * form)
{
    for (int i = 0; i < form->n_fields; i++)
        form->fields[i]->error = "";
}

// Free memory allocated for a list of elements
static void free_arr(void ** arr, int n_elem)
{
    for (int i = 0; i < n_elem; i++)
        free(arr[i]);
    free(arr);
}
