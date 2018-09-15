#include <stdlib.h>
#include <stdio.h>

#include <libgfx.h>

#include "obj.h"
#include "event.h"
#include "box.h"
#include "button.h"
#include "cbm.h"
#include "error.h"
#include "frame.h"
#include "layout-ops.h"
#include "list.h"
#include "message.h"
#include "table.h"
#include "window.h"

#include "bank-allocator.h"
#include "basic-starter.h"
#include "file-window.h"
#include "main.h"
#include "ultimem.h"

struct obj * desktop;
struct obj * focussed_window;

void
shift_charset ()
{
    int i;

    for (i = 0; i < 2048; i++)
        charset_4x8[i] <<= 4;
}

char do_shutdown = 0;

void
init ()
{
    /* Active RAM in BANK5. */
    * (char *) 0x9ff2 = 0xff;

    /* Add memory blocks for malloc(). */
    _heapadd ((void *) 0x400, 0xc00);    /* +3K */
    _heapadd ((void *) 0x9800, 0x7f0);   /* IO2/3 excluding Ultimem registers. */
    _heapadd ((void *) 0xa000, 0x2000);  /* BANK5 */

    init_bank_allocator ();
    gfx_clear_screen (0);
    gfx_init ();
    shift_charset ();
    gfx_set_font (charset_4x8, 2);

    focussed_window = NULL;
    desktop = OBJ(make_box (pattern_woven));
    set_obj_position_and_size (desktop, 0, 0, 20 * 8, 12 * 16 - MESSAGE_HEIGHT);
}

void
show_free_memory ()
{
    char msg[32];

    sprintf (msg, "%d B free.", _heapmemavail ());
    print_message (msg);
}

int
main (int argc, char ** argv)
{
    struct obj * tmp;
    char key;
    struct event * e;
    char msg[16];

    init ();
    append_obj (desktop, tmp = make_file_window ("#8 SD2IEC", 0, 0, 20 * 8, 12 * 16 - MESSAGE_HEIGHT));
//    append_obj (desktop, make_basic_starter ());
    layout_obj (desktop);
    draw_obj (desktop);
    print_message ("Welcome to G - press F1 for help.");

//    for (i = 0; i < 50; i++)
//        gfx_draw_line (0, 0, 120, i);

    do {
//        show_free_memory ();
        while (!(key = cbm_read_char ()));

//        sprintf (msg, "%u B free.", key);
//        print_message (msg);
        /* Send keyboard event. */
        e = malloc (sizeof (struct event));
        e->type = EVT_KEYPRESS;
        e->data_char = key;
        send_event ((struct obj *) focussed_window, e);
        free (e);
    } while (!do_shutdown);

    return 0;
}
