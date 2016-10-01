#ifndef WOODY_COMMON_H
#define WOODY_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define Assert(expression, ...) \
    if (!expression)            \
    {                           \
        printf(##__VA_ARGS__);  \
        exit(1);                \
    }                           \
    else                        \
    {                           \
                                \
    }

#endif
