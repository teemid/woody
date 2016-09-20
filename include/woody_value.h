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


#define Number(tvalue) (tvalue)->value.number
#define Boolean(tvalue) (tvalue)->value.boolean
#define Function(tvalue) (tvalue)->value.function


DECLARE_BUFFER(Value, TaggedValue);


int32_t ValueBufferFind (ValueBuffer * buffer, TaggedValue value);


#endif
