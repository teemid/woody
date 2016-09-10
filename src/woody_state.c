#include "stdlib.h"

#include "woody_state.h"
#include "woody_utils.h"
#include "woody_memory.h"


WoodyState * WoodyNewState ()
{
    WoodyState * state = (WoodyState *)Allocate(sizeof(WoodyState));

    state->code = InstructionBufferNew(40);
    state->constants = ValueBufferNew(40);
    state->stack = Buffer(double, 40);

    state->ip = state->code->values;
    state->stack_ptr = state->stack;
    state->stack_top = state->stack + 40;

    return state;
}
