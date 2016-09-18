#ifndef WOODY_COMMON_H
#define WOODY_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define Assert(expression, message) \
    if (expression)                 \
    {                               \
        printf(message);            \
        exit(1);                    \
    }                               \
    else                            \
    {                               \
                                    \
    }

#endif
