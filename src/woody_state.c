#include "stdlib.h"

#include "woody_state.h"
#include "woody_utils.h"


WoodyState * WoodyNewState ()
{
    WoodyState * state = malloc(sizeof(WoodyState));

    state->code = InstructionBufferNew(40);
    state->constants = ValueBufferNew(40);

    return state;
}
