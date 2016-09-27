#ifndef WOODY_STATE_H
#define WOODY_STATE_H


#include "woody_common.h"
#include "woody_value.h"

typedef int32_t Instruction;
typedef struct WoodyFunction WoodyFunction;
typedef struct TaggedValue TaggedValue;
typedef TaggedValue * StackPtr;


typedef struct
{
    Instruction * ip;
    WoodyFunction * function;
    StackPtr start;
} CallFrame;


typedef struct WoodyState
{
    WoodyFunction * functions;
    StackPtr stack;
    StackPtr current;
    StackPtr top;
    CallFrame * frames;
    uint32_t frame_count;
    uint32_t frame_capacity;
    Object root;
} WoodyState;


WoodyState * WoodyNewState ();
void WoodyFreeState(WoodyState * state);


#endif
