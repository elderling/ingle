#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

char * log_ptr = (char *) 0x400;

void
log_message (char * format, ...)
{
    va_list args;

    va_start(args, format);
    vsprintf (log_ptr, format, args);
    log_ptr += strlen (log_ptr) + 1;
    va_end(args);
}