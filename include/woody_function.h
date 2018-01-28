#ifndef WOODY_FUNCTION_H
#define WOODY_FUNCTION_H

#include "woody_state.h"
#include "woody_common.h"
#include "woody_value.h"
#include "woody_utils.h"


typedef int32_t Instruction;
typedef struct ValueBuffer ValueBuffer;


DECLARE_BUFFER(Instruction, instruction, Instruction);


typedef struct WoodyFunction
{
    Object object;
    struct WoodyFunction * parent;
    struct WoodyFunction ** functions;
    uint32_t function_count;
    uint32_t function_capacity;
    char * name;
    ValueBuffer * constants;
    InstructionBuffer * code;
    uint32_t local_variables;
    uint8_t arity;
} WdyFunction;


typedef struct
{
    bool is_closed;
    TaggedValue * value;
    TaggedValue closed;
} Upvalue;


#define FLEXIBLE_ARRAY 1


typedef struct WoodyClosure
{
    WdyFunction * function;
    Upvalue ** upvalues;
} WdyClosure;


WdyFunction * wdy_function_new(WdyState * state, WdyFunction * parent);
void wdy_function_free(WdyFunction * function);

WdyClosure * wdy_closure_new(WdyFunction * function, uint32_t upvalue_count);


#endif
