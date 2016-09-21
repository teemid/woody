#include "woody_memory.h"
#include "woody_function.h"


DEFINE_BUFFER(Instruction, Instruction);


WoodyFunction * WoodyFunctionNew (WoodyFunction * parent)
{
    WoodyFunction * function = (WoodyFunction *)Allocate(sizeof(WoodyFunction));

    function->parent = NULL;
    function->functions = NULL;
    function->function_count = 0;
    function->function_capacity = 0;
    function->constants = NULL;
    function->code = InstructionBufferNew(20);
    function->arity = 0;

    if (parent)
    {
        WoodyFunctionSetParent(function, parent);
    }

    return function;
}


void WoodyFunctionInitialize (WoodyFunction * function)
{
    function->parent = NULL;
    function->functions = NULL;
    function->function_count = 0;
    function->function_capacity = 0;
    function->constants = NULL;
    function->code = InstructionBufferNew(20);
    function->arity = 0;
}


void WoodyFunctionInitializeConstants (WoodyFunction * function)
{
    function->constants = ValueBufferNew(20);
}


void WoodyFunctionSetParent (WoodyFunction * function, WoodyFunction * parent)
{
    // Set the parent of this function
    function->parent = parent;

    if (!parent->function_capacity)
    {
        uint32_t initial_function_capacity = 5;
        parent->functions = Buffer(WoodyFunction, initial_function_capacity);
    }

    if (parent->function_count == parent->function_capacity)
    {
        uint32_t new_capacity = parent->function_capacity * 2;
        parent->functions = ResizeBuffer(WoodyFunction, parent->functions, new_capacity);
        parent->function_capacity = new_capacity;
    }

    WoodyFunction * child = parent->functions + parent->function_count++;
    child = function;
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
