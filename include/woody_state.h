#ifndef WOODY_STATE_H
#define WOODY_STATE_H

#include "woody_function.h"
#include "woody_value.h"


typedef TaggedValue * StackPtr;


typedef struct WoodyState
{
    Instruction * ip;
    WoodyFunction * function;
    StackPtr stack;
    StackPtr current;
    StackPtr top;
} WoodyState;


WoodyState * WoodyNewState ();
void WoodyFreeState(WoodyState * state);


#endif
