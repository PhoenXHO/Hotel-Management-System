#include "notepad.h"
#include "globals.h"

#define CTRL(x) ((x) & 0x1f)

TEXTAREA * init_textarea(void);

act_result textarea(void)
{
    TEXTAREA * textarea = init_textarea();
    int line_i = 0; // Line index
    short x = 1; // Cursor position relative to `mainwin`
    const short max_size = mainwin->_maxx;

    // Show the cursor if hidden, then move it to the beginning of the first field
    curs_set(1);
    wmove(mainwin, 1, 1);
    wrefresh(mainwin);

    wchar_t ch;
    while (true)
    {
        ch = wgetch(mainwin);

        LINE * line = textarea->lines[line_i];
        switch (ch)
        {
            // Quit when pressing Ctrl + 'q'
            case CTRL('q'): return ACT_RETURN;
            default:
                handle_line(mainwin, line, ch, &x, max_size);
        }

        mvwprintw(mainwin, 1, 1, "%.*s", max_size, line->buffer);
        // Show cursor, then move it to the top-left of the window (textarea)
        curs_set(1);
        wmove(mainwin, 1, x);
        wrefresh(mainwin);
    }

    return ACT_CONTINUE;
}

TEXTAREA * init_textarea(void)
{
    TEXTAREA * textarea = (TEXTAREA *)malloc(sizeof(TEXTAREA));

    textarea->n_lines = 1;
    init_lines(&textarea->lines, textarea->n_lines);

    return textarea;
}
