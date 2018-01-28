#ifndef WOODY_STATE_H
#define WOODY_STATE_H


#include "woody_common.h"
#include "woody_value.h"

typedef int32_t Instruction;
typedef struct WoodyFunction WdyFunction;
typedef struct TaggedValue TaggedValue;
typedef TaggedValue * StackPtr;


typedef struct
{
    Instruction * ip;
    WdyFunction * function;
    StackPtr start;
} CallFrame;


typedef struct WoodyState
{
    WdyFunction * functions;
    StackPtr stack;
    StackPtr current;
    StackPtr top;
    CallFrame * frames;
    uint32_t frame_count;
    uint32_t frame_capacity;
    Object root;
} WdyState;


WdyState * wdy_state_new();
void wdy_state_free(WdyState * state);

#endif
