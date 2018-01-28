#ifndef WOODY_COMMON_H
#define WOODY_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


#define UNUSED(var) (void)(var)

#if defined WOODY_WINDOWS
    void print_to_debug(char * message_format, ...);

    #define LOG(message, ...) print_to_debug(message, __VA_ARGS__)
#else
    #define LOG(message, ...) fprintf(stderr, message, __VA_ARGS__)
#endif


#define ASSERT(expression, message, ...)                                        \
    if (!expression)                                                            \
    {                                                                           \
        LOG("Expression: %s failed at %s:%i", #expression, __FILE__, __LINE__); \
        LOG(message, __VA_ARGS__);                                              \
        exit(1);                                                                \
    }                                                                           \
    else { }

#endif
