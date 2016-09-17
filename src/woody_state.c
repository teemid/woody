#include "woody_memory.h"
#include "woody_state.h"
#include "woody_utils.h"
#include "woody_value.h"


WoodyState * WoodyNewState ()
{
    WoodyState * state = (WoodyState *)Allocate(sizeof(WoodyState));

    state->code = InstructionBufferNew(40);
    state->constants = ValueBufferNew(40);
    state->stack = Buffer(TaggedValue, 40);

    state->function = WoodyFunctionNew();

    state->ip = state->code->values;
    state->stack_ptr = state->stack;
    state->stack_top = state->stack + 40;

    return state;
}
