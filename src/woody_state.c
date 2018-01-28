#include "woody_memory.h"
#include "woody_state.h"


WdyState * wdy_state_new()
{
    WdyState * state = (WdyState *)wdy_allocate(sizeof(WdyState));

    state->stack = NULL;
    state->top = NULL;
    state->current = NULL;
    state->frames = NULL;
    state->frame_count = 0;
    state->frame_capacity = 0;

    return state;
}


void wdy_state_free(WdyState * state)
{
    wdy_deallocate(state->stack);
    wdy_deallocate(state);
}
