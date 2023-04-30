#ifndef NOTEPAD_H_INCLUDED
#define NOTEPAD_H_INCLUDED

#include "utils.h"

typedef struct
{
    LINE ** lines; // List of lines
    int n_lines; // Number of lines
} TEXTAREA;

act_result textarea(void);

#endif // NOTEPAD_H_INCLUDED
