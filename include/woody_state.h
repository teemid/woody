#ifndef WOODY_STATE_H
#define WOODY_STATE_H

#include "woody_function.h"
#include "woody_value.h"


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
} WoodyState;


WoodyState * WoodyNewState ();
void WoodyFreeState(WoodyState * state);


#endif
