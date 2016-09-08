#ifndef WOODY_STATE_H
#define WOODY_STATE_H

#include "woody_utils.h"


typedef struct
{
    Instruction * ip;
    InstructionBuffer * code;
    ValueBuffer * constants;
    ValueBuffer * stack;
    double * stack_ptr;
} WoodyState;


WoodyState * WoodyNewState ();


#endif
