#ifndef TODO_H_INCLUDED
#define TODO_H_INCLUDED

#include "utils.h"

#define MAX_SIZE 53

typedef struct
{
    LINE * content;
    DATE date;
} TASK;

typedef struct
{
    TASK ** tasks;
    int n_tasks;
} TODOLIST;

act_result todo(void);
void add_task_ondate(DATE);
void destroy_todolist(void);

#endif // TODO_H_INCLUDED
