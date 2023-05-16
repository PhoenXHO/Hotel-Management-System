#ifndef NOTEPAD_H_INCLUDED
#define NOTEPAD_H_INCLUDED

#include "utils.h"

#define MAX_SIZE 53

typedef struct
{
    LINE ** lines; // List of lines
    int n_lines; // Number of lines
//    int strstart;
    int linestart;
} TEXTAREA;

typedef struct
{
    LINE ** notes;
    int n_notes;
} NOTELIST;

act_result notepad(void);
void destroy_notelist(void);

#endif // NOTEPAD_H_INCLUDED
