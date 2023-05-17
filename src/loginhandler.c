#include <ctype.h>
#include <string.h>
#include "loginhandler.h"
#include "globals.h"

#define ENCRYPTION_KEY 3

static bool is_valid_email(char *, int length);
static int is_valid_pw(char *, int length);
static int is_valid_username(char *, int length);

static char * encrypt_pw(char *, int length);

static void init_user_data(int username_len, int email_len, int pw_len);

USER * user;

// Log a user in
bool login_user(FORM * form)
{
    LINE * email = form->fields[0]->line;
    LINE * pw = form->fields[1]->line;
    char * encrypted_pw = encrypt_pw(pw->buffer, pw->length);

    bool found_email = false,
         found_pw = false;

    // Open database file
    FILE * db = fopen("user_info.txt", "r");
    fassert(db, "user_info.txt");

    fseek(db, 0, SEEK_END);
    size_t file_size = ftell(db);
    fseek(db, 0, SEEK_SET);
    size_t cur_pos = ftell(db);

    // Look up the user's information in the database
    // Stop if at the end of the file
    //      or if both the email and the password were found
    LINE * stored_username;
    while (cur_pos != file_size && !(found_email || found_pw))
    {
        stored_username = fscans_tab(db);
        char * stored_email = fscans_tab(db)->buffer;
        char * stored_pw = fscans_tab(db)->buffer;

        // Compare emails
        found_email = strcmp(email->buffer, stored_email) == 0;
        // Compare passwords
        found_pw = strcmp(encrypted_pw, stored_pw) == 0;

        cur_pos = ftell(db);
    }

    if (!found_email)
        form->fields[0]->error = "Incorrect email.";
    if (found_email && !found_pw)
        form->fields[1]->error = "Incorrect password.";

    if (!found_email || !found_pw)
    {
        fclose(db);
        return false;
    }

    // Store the user data
    init_user_data(stored_username->length, email->length, pw->length);
    strcpy(user->username, stored_username->buffer);
    strcpy(user->email, email->buffer);
    strcpy(user->enc_pw, encrypted_pw);
    user->new_account = false;

    return true;
}

// Register a new user
bool register_user(FORM * form)
{
    LINE * username = form->fields[0]->line;
    LINE * email = form->fields[1]->line;

    bool found_username = false,
         found_email = false;

    // Open database file
    FILE * db = fopen("user_info.txt", "a+");
    fassert(db, "user_info.txt");

    fseek(db, 0, SEEK_END);
    size_t file_size = ftell(db);
    fseek(db, 0, SEEK_SET);
    size_t cur_pos = ftell(db);

    // Look up the user's information in the database
    // Stop if at the end of the file
    //      or if either the email or the password were found
    while (cur_pos != file_size && !(found_username || found_email))
    {
        char * stored_username = fscans_tab(db)->buffer;
        char * stored_email = fscans_tab(db)->buffer;
        fscans_tab(db); // Discard the password

        // Compare usernames
        found_username = strcmp(username->buffer, stored_username) == 0;
        // Compare emails
        found_email = strcmp(email->buffer, stored_email) == 0;

        cur_pos = ftell(db);
    }

    if (found_username)
        form->fields[0]->error = "Username already exists.";
    if (found_email)
        form->fields[1]->error = "Email already exists.";

    if (found_email || found_username)
    {
        fclose(db);
        return false;
    }

    LINE * pw = form->fields[2]->line;
    char * encrypted_pw = encrypt_pw(pw->buffer, pw->length);

    // Append user information to the database
    rewind(db);
    fprintf(db, "%s\t%s\t%s\n", username->buffer, email->buffer, encrypted_pw);

    // Store the user data
    init_user_data(username->length, email->length, pw->length);
    strcpy(user->username, username->buffer);
    strcpy(user->email, email->buffer);
    strcpy(user->enc_pw, encrypted_pw);
    user->new_account = true;

    fclose(db);
    return true;
}

// Validate all form fields
bool validate_form(FORM * form, bool is_login)
{
    bool is_valid = true;
    int email_i = (int)!is_login,
        length;

    if (!is_login)
    {
        FIELD * username = form->fields[0];
        length = username->line->length;
        int username_res = is_valid_username(username->line->buffer, length);
        if (username_res == 0)
        {
            username->error = "Username too short. (min. " XSTR(USRN_MIN) ")";
            is_valid = false;
        }
        else if (username_res == -1)
        {
            username->error = "Username too long. (max. " XSTR(USRN_MAX) ")";
            is_valid = false;
        }
        else if (username_res == -2)
        {
            username->error = "Invalid username.";
            is_valid = false;
        }
    }

    FIELD * email = form->fields[email_i];
    length = email->line->length;
    if (!is_valid_email(email->line->buffer, length))
    {
        email->error = "Invalid email.";
        is_valid = false;
    }

    FIELD * pw = form->fields[email_i + 1];
    length = pw->line->length;
    int pw_res = is_valid_pw(pw->line->buffer, length);
    if (pw_res == 0)
    {
        if (is_login)
            pw->error = "Incorrect password.";
        else
            pw->error = "Password too short. (min. " XSTR(PW_MIN) ")";
        is_valid = false;
    }
    else if (pw_res == -1)
    {
        pw->error = "Password cannot contain white spaces.";
        is_valid = false;
    }

    return is_valid;
}

// Check validity of an email
static bool is_valid_email(char * email, int length)
{
    if (!isalpha(email[0]) || strchr(email, ' ') != NULL)
        return false;

    int at = -1,
        dot = -1,
        at_count = 0;

    for (int i = 0; i < length; i++)
    {
        char c = email[i];

        if (c == '@')
        {
            at_count++;
            at = i;
            continue;
        }
        else if (c == '.')
        {
            dot = i;
            continue;
        }

        if (!isalpha(c) && !isdigit(c) && c != '_')
            return false;
    }

    return  at_count == 1 && at != -1 &&
            dot != -1 && at < dot - 1 &&
            dot != length - 1;
}

// Check validity of a password
static int is_valid_pw(char * pw, int length)
{
    // Invalid password if it is less than PW_MIN
    if (length < PW_MIN) return 0;
    // Invalid password if it contains spaces
    if (strchr(pw, ' ') != NULL) return -1;

    return 1;
}

// Check validity of a username
static int is_valid_username(char * username, int length)
{
    for (int i = 0; i < length; i++)
        if (!isalpha(username[i]) && !isdigit(username[i]) && username[i] != '_')
            return -2;

    // Invalid username if it is less than USRN_MIN
    if (length < USRN_MIN) return 0;
    // Invalid username if it is longer than USRN_MAX
    if (length > USRN_MAX) return -1;

    return 1;
}

// Encrypt a password
static char * encrypt_pw(char * pw, int length)
{
    char * encrypted = (char *)malloc((length + 1) * sizeof(char));

    // Encrypt the password using a simple Caesar cipher
    for (int i = 0; i < length; i++) {
        char c = pw[i];

        if (isupper(c)) {
            c = ((c - 'A') + ENCRYPTION_KEY) % 26 + 'A';
        } else if (islower(c)) {
            c = ((c - 'a') + ENCRYPTION_KEY) % 26 + 'a';
        } else if (isdigit(c)) {
            c = ((c - '0') + ENCRYPTION_KEY) % 10 + '0';
        }

        encrypted[i] = c;
    }
    encrypted[length] = '\0';

    return encrypted;
}

static void init_user_data(int username_len, int email_len, int pw_len)
{
    user = (USER *)malloc(sizeof(USER));

    user->username = (char *)malloc(username_len * sizeof(char));
    user->email = (char *)malloc(email_len * sizeof(char));
    user->enc_pw = (char *)malloc(pw_len * sizeof(char));
}

void destroy_user(void)
{
    free(user->username);
    free(user->email);
    free(user->enc_pw);
    free(user);
}
