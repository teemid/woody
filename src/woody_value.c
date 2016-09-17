#include "woody_memory.h"
#include "woody_value.h"


extern static char * woody_types[] = {
    "number",
    "boolean",
    "function",
};

DEFINE_BUFFER(Value, TaggedValue);

WoodyFunction * WoodyFunctionNew (void)
{
    WoodyFunction * function = (WoodyFunction *)Allocate(sizeof(WoodyFunction));

    function->parent = NULL;
    function->functions = NULL;
    function->function_count = 0;
    function->function_capacity = 0;
    function->constants = NULL;
    function->code = InstructionBufferNew(20);
    function->arity = 0;

    return function;
}


void WoodyFunctionFree (WoodyFunction * function)
{
    ValueBufferFree(function->constants);
    InstructionBufferFree(function->code);

    if (function->function_capacity)
    {
        Deallocate(function->functions);
    }

    Deallocate(function);
}
