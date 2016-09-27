#ifndef WOODY_FUNCTION_H
#define WOODY_FUNCTION_H

#include "woody_state.h"
#include "woody_common.h"
#include "woody_value.h"
#include "woody_utils.h"


typedef int32_t Instruction;
typedef struct ValueBuffer ValueBuffer;


DECLARE_BUFFER(Instruction, Instruction);


typedef struct WoodyFunction
{
    Object object;
    struct WoodyFunction * parent;
    struct WoodyFunction ** functions;
    uint32_t function_count;
    uint32_t function_capacity;
    ValueBuffer * constants;
    InstructionBuffer * code;
    uint32_t local_variables;
    uint8_t arity;
} WoodyFunction;


typedef struct
{
    bool is_closed;
    TaggedValue * value;
    TaggedValue closed;
} Upvalue;


#define FLEXIBLE_ARRAY 1


typedef struct WoodyClosure
{
    WoodyFunction * function;
    Upvalue ** upvalues;
} WoodyClosure;


WoodyFunction * WoodyFunctionNew (WoodyState * state, WoodyFunction * parent);
void WoodyFunctionFree (WoodyFunction * function);

WoodyClosure * WoodyClosureNew (WoodyFunction * function, uint32_t upvalue_count);


#endif
