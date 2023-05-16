#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "loginhandler.h"
#include "sidebar.h"

#define sidebar_width 32

// Directory of the database
extern const char * db_dir;
extern const short db_dir_len;
extern const char * db_ext;
extern const short db_ext_len;
extern const char * note_ext;
extern const short note_ext_len;

extern char * user_dir;

extern char * tasks_dir;

// The main window
extern WINDOW * mainwin;
extern SIDEBAR * sidebar;

// Dimensions of the main window
extern short mainwin_width;
extern short mainwin_height;

// A global variable to store user data
extern USER * user;

extern bool todo_called;
extern bool notepad_called;

#endif // MAINWINDOW_H_INCLUDED
