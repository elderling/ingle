#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <cbm.h>

#include <lib/ingle/cc65-charmap.h>
#include <lib/term/libterm.h>
#include <lib/lineedit/liblineedit.h>
#include <lib/text/line.h>
#include <lib/text/motion.h>

#include "commands.h"
#include "screen.h"
#include "keyboard.h"

#define CANCELLED   -1
#define OK          0

typedef void (*voidfun) ();

void do_nothing (void) {}

typedef struct _command {
    char     key;
    voidfun  fun;
} command;

// After these edit_mode() is called.
command edit_commands[] = {
    { 'i', do_nothing },
    { 'I', move_line_begin },
    { 'o', cmd_open_below },
    { 'O', cmd_open_above },
    { 'a', move_right },
    { 'A', move_line_end },
    { 'C', cmd_change_till_line_end },
    { 's', cmd_delete_char },
    { 0, NULL }
};

command motion_commands[] = {
    { TTY_CURSOR_LEFT,  move_left },
    { TTY_CURSOR_UP,    move_up },
    { TTY_CURSOR_DOWN,  move_down },
    { TTY_CURSOR_RIGHT, move_right },
    { 'h', move_left },
    { 'k', move_up },
    { 'j', move_down },
    { 'l', move_right },
    { '0', move_line_start },
    { '$', move_line_last_char },
    { 'G', move_last_line },
    { 'w', move_word },
    { 'b', move_word_back },
    { 0, NULL }
};

command track_commands[] = {
    { 'd', cmd_delete },
    { 'c', cmd_change },
    { 'c', cmd_yank },
    { 'c', cmd_paste },
    { 'c', cmd_follow },
    { 0, NULL }
};

command modify_commands[] = {
    { 'D', cmd_delete_till_line_end },
    { 'd', cmd_delete_line },
    { 'x', cmd_delete_char },
    { 'r', cmd_replace_char },
    { 'J', cmd_join },
    { 'p', cmd_paste_below },
    { 'P', cmd_paste_above },
    { 'v', cmd_toggle_visual_mode },
    { 0, NULL }
};

command complex_commands[] = {
    { 'w', cmd_write_file },
    { 'r', cmd_read_file },
    { 'k', cmd_set_passphrase },
    { 'q', cmd_quit },
    { 0, NULL }
};

char
input (void)
{
    char key;

    while (1) {
        switch (key = get_key ()) {
            case TTY_ENTER:
            case TTY_ESCAPE:
                goto done;

            default:
                if (key == TTY_BACKSPACE && !xpos)
                    goto done;
                lineedit (key);
        }
    }

done:
    return key;
}

void
edit_mode (void)
{
    char c;
    screen_set_status ("-- INSERT --");

    while (1) {
        screen_update ();
        line_to_buf ();
        c = input ();
        buf_to_line ();

        switch (c) {
            case TTY_ENTER:
                cmd_enter ();
                continue;

            case TTY_ESCAPE:
                goto done;

            case TTY_BACKSPACE:
                if (linenr) {
                    move_up ();
                    cmd_join ();
                }
                continue;
        }
    }

done:
    move_left ();
    screen_set_status ("");
}

voidfun
get_command_fun (command * cmds, char c)
{
    while (cmds->key) {
        if (cmds->key == c)
            return cmds->fun;
        cmds++;
    }

    return NULL;
}

char
exec_single_command ()
{
    char     c;
    voidfun  fun;

    c = get_key ();

    if (fun = get_command_fun (edit_commands, c)) {
        fun ();
        edit_mode ();
    } else if (fun = get_command_fun (motion_commands, c))
        fun ();
    else if (fun = get_command_fun (modify_commands, c))
        fun ();
    else
        return c;

    return OK;
}

unsigned
get_repetitions (void)
{
    char      c;
    unsigned  n = 0;

    while (1) {
        c = peek_key ();

        if (c == TTY_ESCAPE) {
            get_key ();
            return 0;
        }

        if (!isdigit (c))
            return n;

        if (n > 6553) {
            term_put (TERM_BELL);
            return 0;
        }

        get_key ();
        unlog_key ();
        n *= 10;
        n += c - '0';
    }
}

char exec_action (void);

void
playback (void)
{
    start_playback ();
    exec_single_command ();
    stop_playback ();
}

char msg[256]; // TODO: Remove.

unsigned repetitions = 0;

// Do an action with optional number of repitions prefixed.
char
exec_action ()
{
    unsigned i;

    reset_log ();
    repetitions = 0;

    if (isdigit (peek_key ()))
        repetitions = get_repetitions ();

    if (exec_single_command ())
        goto cancel;

    if (repetitions) {
        sprintf (msg, "%D reps, %D log", repetitions, keylog_index);
        gotoxy (0, rows - 2);
        term_puts (msg);
        for (i = 1; i < repetitions; i++)
            playback ();
    }

    return OK;

cancel:
    term_put (TERM_BELL);
    return CANCELLED;
}

void
exec_complex (void)
{
    unsigned  oldx = xpos;
    char      c;
    voidfun   f;

    lineedit_init ();
    ypos = rows - 1;
    c = input ();
    reset_log ();

    if (c == TTY_ENTER) {
        f = get_command_fun (complex_commands, linebuf[1]);
        if (f)
            f ();
    }

    xpos = oldx;
}

// Top level controlling repetitions (keyboard
// logging and triggering playback).
void
toplevel (void)
{
    char c;

    while (1) {
        c = peek_key ();

        if (c == ':')
            exec_complex ();
        else if (c == '.' && has_logged_keys ()) {
            get_key ();
            unlog_key ();
            playback ();
        } else if (exec_action ())
            keyboard_init ();

        screen_update ();
    }
}

void
our_heapadd (unsigned start, size_t size)
{
    char *  p = (char *) start;
    char    v = *p + 1;

    *p = v;
    if (*p == v)
        _heapadd (p, size);
}

void
main (void)
{
    our_heapadd (0x400, 0xc00);   /* +3K */
    our_heapadd (0xa000, 0x2000); /* BANK5 */

    term_init ();
    line_init ();
    screen_init ();
    keyboard_init ();
    screen_redraw ();
    toplevel ();
}
