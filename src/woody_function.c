#include "woody_function.h"
#include "woody_memory.h"
#include "woody_value.h"


DEFINE_BUFFER(Instruction, Instruction);


WoodyFunction * WoodyFunctionNew (WoodyState * state, WoodyFunction * parent)
{
    WoodyFunction * function = (WoodyFunction *)Allocate(sizeof(WoodyFunction));

    function->object.next = state->root.next;
    state->root.next = &function->object;

    function->parent = parent;
    function->functions = NULL;
    function->function_count = 0;
    function->function_capacity = 0;
    function->constants = ValueBufferNew(4);
    function->code = InstructionBufferNew(20);
    function->local_variables = 0;
    function->arity = 0;

    return function;
}


void WoodyFunctionFree (WoodyFunction * function)
{
    if (function->function_count)
    {
        Deallocate(function->functions);
    }

    InstructionBufferFree(function->code);

    Deallocate(function);
}
