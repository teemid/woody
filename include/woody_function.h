#ifndef WOODY_FUNCTION_H
#define WOODY_FUNCTION_H

#include "woody_common.h"
#include "woody_value.h"
#include "woody_utils.h"


typedef uint32_t Instruction;


DECLARE_BUFFER(Instruction, Instruction);


typedef struct WoodyFunction
{
    struct WoodyFunction * parent;
    struct WoodyFunction * functions;
    uint32_t function_count;
    uint32_t function_capacity;
    ValueBuffer * constants;
    InstructionBuffer * code;
    uint8_t arity;
} WoodyFunction;


WoodyFunction * WoodyFunctionNew (WoodyFunction * parent);
void WoodyFunctionInitialize (WoodyFunction * function);
void WoodyFunctionSetParent (WoodyFunction * function, WoodyFunction * parent);
void WoodyFunctionInitializeConstants (WoodyFunction * function);
void WoodyFunctionFree (WoodyFunction * function);


#endif
