#include "todo.h"
#include "globals.h"
#include "menu.h"

#define menu_height 25
#define menu_width  64

static act_result display_tasks(void);
static act_result add_task(void);
static act_result edit_task(void);
static act_result mark_task(void);
static act_result remove_task(void);
static act_result quit_todo(void);

act_result todo(void)
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
    MENU * menu = create_newmenu(box, 6);

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
    add_button(menu->win, menu->buttons[3], "Mark a task as done", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
    box.ypos += 4;
    add_button(menu->win, menu->buttons[4], "Remove a task", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(CYAN_BG_BLACK));
    box.ypos += 4;
    add_button(menu->win, menu->buttons[5], "Quit", box, COLOR_PAIR(WHITE_BG_BLACK), COLOR_PAIR(RED_BG_WHITE));

    menu->buttons[0]->action = &display_tasks;
    menu->buttons[1]->action = &add_task;
    menu->buttons[2]->action = &edit_task;
    menu->buttons[3]->action = &mark_task;
    menu->buttons[4]->action = &remove_task;
    menu->buttons[5]->action = &quit_todo;

    handle_menu(menu);

    wrefresh(mainwin);

    return ACT_RETURN;
}

static act_result display_tasks(void)
{
    return ACT_CONTINUE;
}

static act_result add_task(void)
{
    return ACT_CONTINUE;
}

static act_result edit_task(void)
{
    return ACT_CONTINUE;
}

static act_result mark_task(void)
{
    return ACT_CONTINUE;
}

static act_result remove_task(void)
{
    return ACT_CONTINUE;
}

static act_result quit_todo(void)
{
    return ACT_RETURN;
}
