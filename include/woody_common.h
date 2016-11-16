#ifndef WOODY_COMMON_H
#define WOODY_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define UNUSED(var) (void)(var)

#if defined WOODY_WINDOWS
    void PrintToDebug(char * message_format, ...);

    #define Log(message, ...) PrintToDebug(message, __VA_ARGS__)
#else
    #define Log(message, ...) fprintf(stderr, message, __VA_ARGS__)
#endif


#define Assert(expression, message, ...)                                        \
    if (!expression)                                                            \
    {                                                                           \
        Log("Expression: %s failed at %s:%i", #expression, __FILE__, __LINE__); \
        Log(message, __VA_ARGS__);                                              \
        exit(1);                                                                \
    }                                                                           \
    else { }

#endif
