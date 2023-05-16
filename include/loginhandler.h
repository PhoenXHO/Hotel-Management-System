#ifndef LOGINHANDLER_H_INCLUDED
#define LOGINHANDLER_H_INCLUDED

#include <stdbool.h>
#include "form.h"

#define PW_MIN      8
#define USRN_MIN    4
#define USRN_MAX    8

typedef struct
{
    char * username;
    char * email;
    char * enc_pw;
    bool new_account;
} USER;

bool validate_form(FORM *, bool is_login);

bool login_user(FORM *);
bool register_user(FORM *);

void destroy_user(void);

#endif // LOGINHANDLER_H_INCLUDED
