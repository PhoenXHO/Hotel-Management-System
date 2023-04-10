#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>
#include <curses.h>

// Utility macros
#define GROW_CAPACITY(cap) \
    (cap) * 1.5
#define GROW_ARRAY(type, arr, cap) \
    (type *)reallocate(arr, GROW_CAPACITY(cap) * sizeof(type))

// Utility functions
WINDOW * create_newwin(int height, int width, int starty, int startx, int type);
void destroy_win(WINDOW *);

void ref_mvwaddch(WINDOW *, int starty, int startx, wchar_t);

void * reallocate(void *, size_t new_cap);

#endif // UTILS_H_INCLUDED
