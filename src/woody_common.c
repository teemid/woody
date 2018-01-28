#include <stdarg.h>
#include <stdio.h>

#include "woody_common.h"
#include "woody_memory.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void print_to_debug(char * message_format, ...)
{
    va_list args;
    va_start(args, message_format);

    int length = vsnprintf(NULL, 0, message_format, args);
    char * buffer = wdy_allocate_buffer(char, length);
    vsprintf(buffer, message_format, args);

    OutputDebugString(buffer);
}
