#include "woody_memory.h"
#include "woody_function.h"


DEFINE_BUFFER(Instruction, Instruction);


WoodyFunction * WoodyFunctionNew (uint32_t initial_stack_size)
{
    WoodyFunction * function = (WoodyFunction *)Allocate(sizeof(WoodyFunction));

    function->parent = NULL;
    function->functions = NULL;
    function->function_count = 0;
    function->function_capacity = 0;
    function->constants = NULL;
    function->code = InstructionBufferNew(initial_stack_size);
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
