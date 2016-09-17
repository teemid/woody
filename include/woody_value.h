#ifndef WOODY_VALUE_H
#define WOODY_VALUE_H

#include "woody_utils.h"


typedef enum
{
    WOODY_NUMBER,
    WOODY_BOOLEAN,
    WOODY_FUNCTION,
} WoodyType;

extern static char * woody_types[];

typedef struct WoodyFunction WoodyFunction;

typedef union
{
    double number;
    uint32_t boolean;
    WoodyFunction * function;
} WoodyValue;


typedef struct
{
    WoodyValue value;
    WoodyType type;
} TaggedValue;


DECLARE_BUFFER(Value, TaggedValue);


struct WoodyFunction
{
    WoodyFunction * parent;
    WoodyFunction * functions;
    uint32_t function_count;
    uint32_t function_capacity;
    ValueBuffer * constants;
    InstructionBuffer * code;
    uint8_t arity;
};


WoodyFunction * WoodyFunctionNew (void);
void WoodyFunctionFree (WoodyFunction * function);


#endif
