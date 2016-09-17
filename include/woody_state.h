#ifndef WOODY_STATE_H
#define WOODY_STATE_H

#include "woody_value.h"


typedef struct
{
    Instruction * ip;
    InstructionBuffer * code;
    ValueBuffer * constants;
    WoodyFunction * function;
    TaggedValue * stack;
    TaggedValue * stack_ptr;
    TaggedValue * stack_top;
} WoodyState;


WoodyState * WoodyNewState ();


#endif
