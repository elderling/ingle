#include <stdlib.h>

#include "obj.h"
#include "window.h"
#include "libgfx.h"

typedef unsigned short ushort;
typedef unsigned char uchar;

#define WHITE   1

#define CONFIG_TYPE     1

void __fastcall__
free_obj (struct obj * x)
{
    struct obj * c;

    while (x) {
        c = x->node.children;
        if (c)
            free_obj (c);
        free (x);
        x = x->node.next;
    }
}

void __fastcall__
draw_obj (struct obj * x)
{
    while (x) {
        x->draw (x);
        x = x->node.next;
    }
}

void
draw_scrollable (struct scrollable * x)
{}

void
draw_message (char * txt)
{}

void probe_devices ()
{
    draw_message ("Probing devices...");
}

void seek_and_load_config ()
{
    draw_message ("Seeking config...");
}

short devices;
struct window * wleft;
struct window * wright;

void
init_filewindow (struct window * w, char device)
{

    /* Make window. */
    /* Draw window. */
    /* Load directory listing. */
    /* Draw window content. */

	wleft = make_window (0, 0, 81, 176, "#8");
	draw_window (wleft);
    wright = make_window (80, 0, 80, 176, "#1");
    draw_window (wright);
}

void
draw_background ()
{
    gfx_reset_region ();
    gfx_set_pattern (pattern_gray);
    gfx_draw_box (0, 0, 20 * 8, 12 * 16);
}

void
win_basic_start ()
{
	struct window * win = make_window (0, 0, 160, 176, "Start BASIC...");

	draw_window (win);
    gfx_set_pattern (pattern_solid);
    gfx_draw_text (2, 50, "( ) no extra memory");
    gfx_draw_text (2, 60, "[ ] +3K");
    gfx_draw_text (2, 70, "( ) +8K");
    gfx_draw_text (2, 80, "( ) +16K");
    gfx_draw_text (2, 90, "( ) +24K");
}

int
main (int argc, char ** argv)
{
    gfx_init ();
    draw_background ();
    win_basic_start ();

    while (1);
    probe_devices ();
    seek_and_load_config ();

    return 0;
}