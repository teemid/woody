#ifndef WOODY_STATE_H
#define WOODY_STATE_H

#include "woody_utils.h"


typedef struct
{
    Instruction * ip;
    InstructionBuffer * code;
    ValueBuffer * constants;
    double * stack;
    double * stack_ptr;
    double * stack_top;
} WoodyState;


WoodyState * WoodyNewState ();


#endif
