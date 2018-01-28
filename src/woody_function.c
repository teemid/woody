#include "woody_function.h"
#include "woody_memory.h"
#include "woody_value.h"


DEFINE_BUFFER(Instruction, instruction, Instruction);


WdyFunction * wdy_function_new(WdyState * state, WdyFunction * parent)
{
    WdyFunction * function = (WdyFunction *)wdy_allocate(sizeof(WdyFunction));

    function->object.next = state->root.next;
    state->root.next = &function->object;

    function->parent = parent;
    function->functions = NULL;
    function->function_count = 0;
    function->function_capacity = 0;
    function->constants = value_buffer_new(4);
    function->code = instruction_buffer_new(20);
    function->local_variables = 0;
    function->arity = 0;

    return function;
}


void wdy_function_free(WdyFunction * function)
{
    if (function->function_count)
    {
        wdy_deallocate(function->functions);
    }

    instruction_buffer_free(function->code);

    wdy_deallocate(function);
}
