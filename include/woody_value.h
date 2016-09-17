#ifndef WOODY_VALUE_H
#define WOODY_VALUE_H

#include "woody_utils.h"


// Forward declaration
typedef struct WoodyFunction WoodyFunction;


typedef enum
{
    WOODY_NUMBER,
    WOODY_BOOLEAN,
    WOODY_FUNCTION,
} WoodyType;

extern const char * woody_types[];

typedef union
{
    double number;
    uint32_t boolean;
    WoodyFunction * function;
} WoodyValue;


typedef struct TaggedValue
{
    WoodyValue value;
    WoodyType type;
} TaggedValue;


DECLARE_BUFFER(Value, TaggedValue);


#endif
