#include <string.h>
#include <unistd.h>
#include "todo.h"
#include "globals.h"
#include "menu.h"
#include "calendar.h"

#define menu_height 21
#define menu_width  64

#define tasks_height    24
#define tasks_width     84

#define tasks_startx    ((mainwin_width) - (tasks_width)) / 2
#define tasks_starty    6

bool todo_called;

TODOLIST * todo_list;
FILE * ftasks;

char * tasks_dir;

static void save_tasks(void);
static void load_tasks(void);

static void init_todolist(void);

static act_result editable_task(int);
static act_result uneditable_task(int);
static act_result display_task(int, bool editable);
static act_result delete_task(int);

static act_result display_tasks(event, chtype highlight);
static act_result display_all_tasks(void);
static act_result add_task(void);
static act_result edit_task(void);
static act_result remove_task(void);

static void add_task_tolist(char *, DATE);

static void refresh_list_win(void);

static void refresh_task(TASK *, WINDOW **, FIELD *, BUTTON *, bool editable);
static void update_tasks(BUTTON ***, event, chtype highlight, int * n_buttons);

act_result todo(void)
{
    todo_called = true;

    tasks_dir = create_newdir("tasks", db_ext);

    init_todolist();
    load_tasks();

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

            add_button(mainwin, &tmp, "To-do List", box, COLOR_PAIR(WHITE_BG_BLACK), 0);
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
        add_button(menu->win, menu->buttons[0], "Display all tasks", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[1], "Add a new task", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[2], "Edit a task", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[3], "Remove a task", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
        box.ypos += 4;
        add_button(menu->win, menu->buttons[4], "Quit", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(RED_BG_WHITE));

        menu->buttons[0]->action = &display_all_tasks;
        menu->buttons[1]->action = &add_task;
        menu->buttons[2]->action = &edit_task;
        menu->buttons[3]->action = &remove_task;
        menu->buttons[4]->action = &__quit__;

        act_result result = handle_menu(menu);
        wrefresh(mainwin);

        if (result == ACT_RETURN) return ACT_RETURN;
        if (result == ACT_CONTINUE) continue;
    }

    return ACT_RETURN;
}

static void init_todolist(void)
{
    todo_list = (TODOLIST *)calloc(1, sizeof(TODOLIST));
}

void destroy_todolist(void)
{
    if (!todo_called) return;
    free_arr((void **)todo_list->tasks, todo_list->n_tasks);
    free(todo_list);
    free(tasks_dir);
}

static act_result editable_task(int index)
{
    return display_task(index, true);
}

static act_result uneditable_task(int index)
{
    return display_task(index, false);
}

static act_result delete_task(int index)
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
        todo_list->n_tasks--;

        destroy_line(todo_list->tasks[index]->content);
        free(todo_list->tasks[index]);
        if (index < todo_list->n_tasks)
            memmove(&todo_list->tasks[index], &todo_list->tasks[index + 1], sizeof(TASK *) * (todo_list->n_tasks - index));

        if (todo_list->n_tasks >= 1)
            todo_list->tasks = SHRINK_ARRAY(TASK *, todo_list->tasks, todo_list->n_tasks);

        save_tasks();

        box = (dim_box){
            .width = MAX_SIZE,
            .xpos = -1,
            .ypos = (mainwin_height - 7) / 2
        };
        box.xpos = sidebar_width + (mainwin_width - box.width) / 2;

        reset_mainwin();
        ALERT * alert = create_newalert(box, "Task removed successfully!", COLOR_PAIR(GREEN_BG_BLACK));

        handle_alert(alert);

        return ACT_UPDATE;
    }
}

static void refresh_task(TASK * task, WINDOW ** win, FIELD * text_field, BUTTON * date_button, bool editable)
{
    dim_box box = {
        .height = 3,
        .width = MAX_SIZE + 4,
        .xpos = sidebar_width + (mainwin_width - (MAX_SIZE + 4)) / 2 + 2,
        .ypos = (mainwin_height - 11) / 2 + 2
    };

    *win = create_newwin(11, box.width + 4, (mainwin_height - 11) / 2, sidebar_width + (mainwin_width - box.width) / 2, 1, COLOR_PAIR(CYAN));

    if (editable)
    {
        wattron(mainwin, COLOR_PAIR(GREEN));
        mvwprintw(mainwin, box.ypos + 11 - 2, box.xpos - sidebar_width + 1, "CTRL + S: Save\t\tENTER: Select/Confirm");
        wattroff(mainwin, COLOR_PAIR(GREEN));
    }
    mvwprintw(mainwin, box.ypos + 11 - 1, box.xpos - sidebar_width + 1, "ARROWS: Navigate");
    mvwprintw(mainwin, box.ypos + 11, box.xpos - sidebar_width + 1, "CTRL + Q: Quit/Cancel");
    wrefresh(mainwin);

    char * date_str = (char *)malloc(11 * sizeof(char));

    DATE date = task->date;
    sprintf(date_str, "%.2d/%.2d/%d", date.day, date.month, date.year);

    box.ypos += 4;

    add_field(mainwin, text_field, "Task description :", box, false, COLOR_PAIR(CYAN));

    mvwprintw(*win, 2, 3, "Task due date :");

    box = (dim_box){
        .height = 1,
        .width = 12,
        .xpos = 4,
        .ypos = 4
    };

    add_button(*win, date_button, date_str, box, 0, 0);

    wrefresh(*win);
}

static act_result display_task(int index, bool editable)
{
    TASK * task = todo_list->tasks[index];

    reset_mainwin();

    WINDOW * prompt;

    FIELD * text_field = create_newfield();
    BUTTON * date_button = (BUTTON *)malloc(sizeof(BUTTON));

    refresh_task(task, &prompt, text_field, date_button, editable);

    if (editable)
        date_button->action = (event)(&get_from_calendar);
    else
        date_button->action = &__continue__;

    short x = 1;
    short max_size = MAX_SIZE + 1;
    bool in_field = true;
    bool quit = false;

    wchar_t ch;
    LINE * line = (LINE *)calloc(1, sizeof(LINE));

    line->length = task->content->length;
    line->capacity = GET_CAPACITY(task->content->length);
    line->buffer = (char *)malloc(line->capacity * sizeof(char));

    strncpy(line->buffer, task->content->buffer, task->content->length + 1);

    mvwprintw(text_field->win, 1, 1, "%.*s", max_size, line->buffer);

    // Show the cursor if hidden, then move it to the beginning of the first field
    curs_set(1);
    wmove(text_field->win, 1, 1);
    wrefresh(text_field->win);

    #define highlight_b() \
        change_button_style(&date_button, prompt, 1, 0, date_button->highlight)
    #define reset_b() \
        change_button_style(&date_button, prompt, 1, 0, date_button->style)

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

            case KEY_UP: case KEY_DOWN:
                if (in_field)
                {
                    curs_set(0);
                    in_field = false;
                    highlight_b();
                }
                else
                {
                    curs_set(1);
                    in_field = true;
                    reset_b();
                    x = ((line->length + 1) > max_size) ? (max_size + 1) : (line->length + 1);
                    line->curs_pos = line->length;
                    line->strstart = ((line->length + 1) > (max_size)) ? (line->length - max_size) : 0;
                }
                break;

            case '\n':
                if (!in_field && editable)
                {
                    fld_clrerr(text_field);
                    reset_mainwin();
                    DATE date = ((d_event)date_button->action)();
                    if (date.day > 0)
                    {
                        task->date = (DATE){
                            .day = date.day,
                            .month = date.month,
                            .year = date.year
                        };
                    }
                    werase(prompt);
                    destroy_win(prompt);
                    reset_mainwin();
                    refresh_task(task, &prompt, text_field, date_button, editable);
                    highlight_b();
                }
                else if (editable)
                {
                    if (line->length == 0)
                    {
                        text_field->error = "Task cannot be empty.";
                        fld_printerr(text_field);
                    }
                    else
                        goto jump;
                }
                break;

            case CTRL('s'):
                if (editable)
                {
                    if (line->length == 0)
                    {
                        text_field->error = "Task cannot be empty.";
                        fld_printerr(text_field);
                    }
                    else
                        goto jump;
                }
                break;

            case CTRL('q'):
                quit = true;
                goto jump;

            // Ignore the following keys
            case KEY_IC: // Insert key
            case KEY_DC: // Delete key
            case KEY_HOME: // Home key
            case KEY_END: // End key
            case KEY_NPAGE: // Page down key
            case KEY_PPAGE: // Page up key
                break;

            default:
                if (editable && in_field)
                    handle_line(text_field->win, line, ch, &x, max_size, 1);
                break;
        }

        mvwprintw(text_field->win, 1, 1, "%.*s", max_size, &line->buffer[line->strstart]);
        wmove(text_field->win, 1, x);
        wrefresh(text_field->win);
    }

    #undef highlight_b
    #undef reset_b

jump:
    line->strstart = line->curs_pos = 0;

    fld_clrerr(text_field);

    destroy_button(date_button);
    destroy_field(text_field);
    destroy_win(prompt);

    if (editable && !quit)
    {
        task->content->length = line->length;
        if (line->length >= task->content->capacity)
        {
            task->content->capacity = GET_CAPACITY(line->length);
            task->content->buffer = GROW_ARRAY(char, task->content->buffer, task->content->capacity);
        }

        strncpy(task->content->buffer, line->buffer, line->length + 1);
        save_tasks();

        dim_box box = (dim_box){
            .width = MAX_SIZE,
            .xpos = -1,
            .ypos = (mainwin_height - 7) / 2
        };
        box.xpos = sidebar_width + (mainwin_width - box.width) / 2;

        reset_mainwin();
        ALERT * alert = create_newalert(box, "Task edited successfully!", COLOR_PAIR(GREEN_BG_BLACK));

        handle_alert(alert);
    }

    free(line->buffer);
    free(line);

    reset_mainwin();
    curs_set(0);

    return ACT_UPDATE;
}

static void update_tasks(BUTTON *** buttons, event e, chtype highlight, int * n_buttons)
{
    if (n_buttons != NULL)
        *n_buttons = todo_list->n_tasks;
    if (todo_list->n_tasks == 0)
        return;

    init_buttons(buttons, todo_list->n_tasks);

    dim_box box = {
        .height = 3,
        .width = tasks_width,
        .xpos = -1,
        .ypos = 0
    };

    for (int i = 0; i < todo_list->n_tasks; i++)
    {
        TASK * task = todo_list->tasks[i];
        DATE date = task->date;

        char * tmp = (char *)malloc((tasks_width + 1) * sizeof(char));
        sprintf(tmp, "%6d %5.2d/%.2d/%.4d", i + 1, date.day, date.month, date.year);
        short curr_len = 5 + 1 + 4 + 10;

        #define OFFSET  4
        #define CUT     10

        if (curr_len + OFFSET + task->content->length <= tasks_width - CUT)
        {
            append_offset(tmp, task->content->buffer, OFFSET, curr_len, task->content->length);
            append_spaces(tmp, tasks_width - (curr_len + OFFSET + task->content->length), curr_len + OFFSET + task->content->length);
        }
        else
        {
            append_offset(tmp, task->content->buffer, OFFSET, curr_len, tasks_width - CUT - (curr_len + OFFSET));
            strcat(tmp, "...");
            append_spaces(tmp, CUT - 3, tasks_width - (CUT - 3));
        }

        #undef OFFSET
        #undef CUT

        create_newbutton((*buttons)[i], tmp, box, 0, highlight);
        (*buttons)[i]->action = e;
    }
}

static act_result display_tasks(event e, chtype highlight)
{
    reset_mainwin();

    WINDOW * tasks_win = create_newwin(tasks_height, tasks_width, tasks_starty, sidebar_width + tasks_startx, -1, 0);

    refresh_list_win();

    if (todo_list->n_tasks > 0)
    {
        BUTTON ** task_buttons;
        update_tasks(&task_buttons, e, highlight, NULL);

        handle_list(tasks_win, &task_buttons, todo_list->n_tasks, tasks_height / 4, &refresh_list_win, &update_tasks);

        free_arr((void **)task_buttons, todo_list->n_tasks);
    }

    if (todo_list->n_tasks == 0)
    {
        reset_mainwin();
        refresh_list_win();

        mvwprintw(tasks_win, 2, (tasks_width - 17) / 2, "No tasks available.");
        wrefresh(tasks_win);
        while (true)
        {
            wchar_t ch = wgetch(tasks_win);

            if (ch == CTRL('q')) break;
        }
    }

    reset_mainwin();
    destroy_win(tasks_win);

    return ACT_CONTINUE;
}

static act_result display_all_tasks(void)
{
    return display_tasks(&uneditable_task, 0);
}

static act_result add_task(void)
{
    reset_mainwin();
    DATE date = get_from_calendar();

    if (date.day <= 0)
        goto quit;

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
    add_field(mainwin, text_field, "Enter a description of your task :", box, false, COLOR_PAIR(CYAN));

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
                    text_field->error = "Task cannot be empty.";
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
    if (line->length > 0)
    {
        add_task_tolist(line->buffer, date);
        save_tasks();
    }

    box = (dim_box){
        .width = MAX_SIZE,
        .xpos = -1,
        .ypos = (mainwin_height - 7) / 2
    };
    box.xpos = sidebar_width + (mainwin_width - box.width) / 2;

    reset_mainwin();
    ALERT * alert = create_newalert(box, "Task added successfully!", COLOR_PAIR(GREEN_BG_BLACK));

    handle_alert(alert);

    destroy_win(prompt);
    destroy_field(text_field);

quit:
    curs_set(0);

    return ACT_CONTINUE;
}

static act_result edit_task(void)
{
    return display_tasks(&editable_task, 0);
}

static act_result remove_task(void)
{
    return display_tasks(&delete_task, COLOR_PAIR(RED_BG_WHITE));
}

static void add_task_tolist(char * content, DATE date)
{
    int length = strlen(content),
        capacity = GET_CAPACITY(length);

    todo_list->n_tasks++;
    todo_list->tasks = GROW_ARRAY(TASK *, todo_list->tasks, todo_list->n_tasks);

    TASK * task = todo_list->tasks[todo_list->n_tasks - 1] = (TASK *)malloc(sizeof(TASK));

    task->content = create_newline();

    task->content->length = length;
    task->content->capacity = capacity;

    task->content->buffer = (char *)malloc((capacity) * sizeof(char));

    strncpy(task->content->buffer, content, length + 1);

    task->date.day = date.day;
    task->date.month = date.month;
    task->date.year = date.year;
}

static void save_tasks(void)
{
    ftasks = fopen(tasks_dir, "w");
    fassert(ftasks, tasks_dir);

    for (int i = 0; i < todo_list->n_tasks; i++)
    {
        TASK * task = todo_list->tasks[i];
        fprintf(ftasks, "%d/%d/%d;%s\n", task->date.day, task->date.month, task->date.year, task->content->buffer);
    }

    fclose(ftasks);
}

static void load_tasks(void)
{
    if (access(tasks_dir, R_OK) != 0)
        return;

    ftasks = fopen(tasks_dir, "r");
    fassert(ftasks, tasks_dir);

    while (!feof(ftasks))
    {
        DATE date;

        char * string = dfscans(ftasks);
        if (!(*string))
            break;

        sscanf(strsep(&string, ";"), "%hd%*c%hd%*c%hd", &date.day, &date.month, &date.year);

        add_task_tolist(string, date);
    }

    fclose(ftasks);
}

static void refresh_list_win(void)
{
    mvwprintw(mainwin, 4, tasks_startx + 5, "ID");
    mvwprintw(mainwin, 4, tasks_startx + 12, "DATE");
    mvwprintw(mainwin, 4, tasks_startx + 24, "CONTENT");

    for (int i = 0; i < tasks_width; i++)
        mvwaddch(mainwin, 5, tasks_startx + i, H_LINE);

    mvwprintw(mainwin, tasks_starty + tasks_height + 1, tasks_startx + 2, "ARROWS: Navigate");
    mvwprintw(mainwin, tasks_starty + tasks_height + 2, tasks_startx + 2, "ENTER: Select");
    mvwprintw(mainwin, tasks_starty + tasks_height + 3, tasks_startx + 2, "CTRL + Q: Quit");

    wrefresh(mainwin);
}
