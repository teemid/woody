#include "woody_memory.h"
#include "woody_state.h"


WoodyState * WoodyNewState ()
{
    WoodyState * state = (WoodyState *)Allocate(sizeof(WoodyState));

    state->stack = NULL;
    state->top = NULL;
    state->current = NULL;
    state->frames = NULL;
    state->frame_count = 0;
    state->frame_capacity = 0;

    return state;
}


void WoodyStateFree (WoodyState * state)
{
    Deallocate(state->stack);

    Deallocate(state);
}
