#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE_LENGTH     256
//#include "jtline.h"

int linenr;
unsigned num_lines;

typedef struct _line line;

typedef struct _line {
    line      * prev;
    line      * next;
    char      * data;
    unsigned  length;
} line;


typedef unsigned pos_t;

unsigned char xpos;
char linebuf[MAX_LINE_LENGTH + 1];
unsigned linebuf_length;

extern void linebuf_clear (void);
extern void linebuf_insert_char (pos_t, char);
extern void linebuf_delete_char (pos_t);
extern void linebuf_replace_char (pos_t, char);
line * first_line;
line * current_line;

line *               line_alloc (void);

void                 line_init           (void);
line *               line_get            (unsigned);
void                 buf_to_line         (void);
void                 line_line_to_buf    (void);
void    line_goto           (unsigned);
void                 line_delete         (void);
void                 line_insert_before  (void);
void                 line_insert_after   (void);
void                 line_split          (void);
void                 line_join           (void);
void                 line_clear          (void);
void move_up (void);
void move_down (void);
void    adjust_xpos_to_line_length (void);


line *
line_alloc (void)
{

    line * l = malloc (sizeof(line));

    l->data = NULL;
    l->prev = NULL;
    l->next = NULL;

    l->length = 0;

    return l;
}

void
line_insert_before (void)
{
    line * new = line_alloc ();

    num_lines++;

    new->prev = current_line->prev;
    new->next = current_line;
    current_line->prev = new;

    if (current_line == first_line)
        first_line = new;

    line_goto (linenr);
}

void
line_insert_after ()
{
    line * new = line_alloc ();

    num_lines++;

    new->prev = current_line;
    new->next = current_line->next;
    current_line->next = new;
}

void
line_delete ()
{
    line * prev = current_line->prev;
    line * next = current_line->next;

    num_lines--;

    if (prev)
        prev->next = next;
    else
        first_line = next;

    free (current_line->data);
    free (current_line);

    if (next) {
        next->prev = prev;
        current_line = next;
    }
}

line *
line_get (unsigned i)
{
    line * l = first_line;

    while (l && i--)
        l = l->next;

    return l;
}

void
buf_to_line ()
{
    char * data;

    free (current_line->data);
    data = malloc (linebuf_length);
    current_line->length = linebuf_length;
    current_line->data = data;
    memcpy (data, linebuf, linebuf_length);
}

void
line_goto (unsigned n)
{
    current_line = line_get (n);
}

void
line_line_to_buf ()
{
    memcpy (linebuf, current_line->data, current_line->length);
    linebuf_length = current_line->length;
}

void
line_split ()
{
    line *  new;
    char *  upper_data;

    line_insert_after ();
    new = current_line->next;
    new->length = current_line->length - xpos;
    new->data = malloc (new->length);
    memcpy (new->data, &current_line->data[xpos], new->length);

    upper_data = malloc (xpos);
    memcpy (upper_data, current_line->data, xpos);
    free (current_line->data);
    current_line->data = upper_data;
    current_line->length = xpos;
}

void
line_join ()
{
    line *    this = current_line;
    line *    next = this->next;
    unsigned  new_len;
    char *    new_data;

    if (!next)
        return;

    new_len = this->length + next->length;
    new_data = malloc (new_len); // TODO: Barf on low memory.

    memcpy (new_data, this->data, this->length);
    memcpy (&new_data[this->length], next->data, next->length);

    free (this->data);
    this->data = new_data;
    this->length = new_len;

    move_down ();
    line_delete ();
    move_up ();
}

void
line_init ()
{
    first_line = current_line = line_alloc ();
    linenr = 0;
    num_lines = 1;
}

void
line_clear ()
{
    line * l = first_line;

    while (l) {
        free (l->data);
        free (l);
        l = l->next;
    }

    line_init ();
}

void
move_up (void)
{
    if (linenr)
        --linenr;

    adjust_xpos_to_line_length ();
}

void
move_down (void)
{
    linenr++;
    if (linenr >= num_lines)
        linenr = num_lines - 1;

    adjust_xpos_to_line_length ();

}

void
adjust_xpos_to_line_length ()
{
    line_goto (linenr);

    if (xpos >= current_line->length)
        xpos = current_line->length ?
            current_line->length - 1 :
            0;
}

void main(void) {
  printf("Beginning tests ...\n");
}
