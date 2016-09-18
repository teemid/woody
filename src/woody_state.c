#include "woody_memory.h"
#include "woody_state.h"


WoodyState * WoodyNewState (uint32_t initial_stack_size)
{
    WoodyState * state = (WoodyState *)Allocate(sizeof(WoodyState));

    state->stack = Buffer(TaggedValue, initial_stack_size);
    state->top = state->stack + initial_stack_size;
    state->current = NULL;

    state->function = WoodyFunctionNew(20);

    state->ip = NULL;

    return state;
}


void WoodyStateFree (WoodyState * state)
{
    Deallocate(state->stack);

    WoodyFunctionFree(state->function);

    Deallocate(state);
}
