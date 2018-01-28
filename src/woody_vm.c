#include <stdio.h>

#include "woody_common.h"
#include "woody_function.h"
#include "woody_memory.h"
#include "woody_opcodes.h"
#include "woody_state.h"
#include "woody_value.h"
#include "woody_vm.h"


#define POP(state) --(state)->current
#define PUSH(state, value) *(state)->current++ = (value)

#define PUSH_NUMBER(state, number_value)       \
    *state->current = MakeNumber(number_value); \
    ++(state)->current

#define PUSH_UNDEFINED(state) \
    (state)->current->value.type = WOODY_UNDEFINED; \
    ++(state)->current

#define CURRENT_FRAME(state) ((state)->frames + (state)->frame_count - 1)
#define CONSTANTS(state) CURRENT_FRAME(state)->function->constants


static void print_stack(WdyState * state)
{
    LOG("Stack: ");

    for (StackPtr i = state->stack; i < state->current; i++)
    {
        switch (i->type)
        {
            case TYPE_NUMBER:
            {
                LOG("%f ", i->value.number);
            } break;
            case TYPE_FALSE:
            {
                LOG("false ");
            } break;
            case TYPE_TRUE:
            {
                LOG("true ");
            } break;
            case TYPE_FUNCTION:
            {
                LOG("Function: %p ", i->value.function);
            } break;
            default:
            {
                LOG("\nError illegal value.");
            } break;
        }
    }

    LOG("\n");
}


static void check_stack(WdyState * state)
{
    if (state->stack == state->top)
    {

    }
}


static void load_constants(WdyState * state, uint32_t index)
{
    TaggedValue value = CONSTANTS(state)->values[index];

    state->current->value = value.value;
    state->current->type = value.type;

    state->current++;
}


static void do_arithmetic(WdyState * state, Instruction i)
{
    TaggedValue * a = POP(state);
    TaggedValue * b = POP(state);

    ASSERT(wdy_is_number(a) && wdy_is_number(b), "Trying arithmetic on non-number types");

    double result = 0;

    switch (i)
    {
        case OP_PLUS:
        {
            result = wdy_number(a) + wdy_number(b);
        } break;
        case OP_MINUS:
        {
            result = wdy_number(b) - wdy_number(a);
        } break;
        case OP_MULT:
        {
            result = wdy_number(a) * wdy_number(b);
        } break;
        case OP_DIV:
        {
            result = wdy_number(b) / wdy_number(a);
        } break;
        default:
        {
            ASSERT(false, "Illegal arithmetic operation\n");
        } break;
    }

    PUSH_NUMBER(state, result);
}


static void initialize_stack(WdyState * state, uint32_t initial_stack_size)
{
    state->stack = wdy_allocate_buffer(TaggedValue, initial_stack_size);
    state->current = state->stack;
    state->top = state->stack + initial_stack_size;

    state->current = state->stack;
}


static void allocate_call_frames(WdyState * state)
{
    uint32_t initial_frame_capacity = 5;
    state->frames = wdy_allocate_buffer(CallFrame, initial_frame_capacity);
    state->frame_count = 0;
    state->frame_capacity = initial_frame_capacity;
}


static void call(WdyState * state, WdyFunction * function)
{
    CallFrame * frame = state->frames + state->frame_count++;
    frame->function = function;
    frame->ip = function->code->values;
    frame->start = state->current;

    // state->current = state->current + frame->function->local_variables;
    for (uint32_t i = 0; i < frame->function->local_variables; i++)
    {
        PUSH_NUMBER(state, 0.0f);
    }
}


static void wdy_return(WdyState * state)
{
    CallFrame * frame = CURRENT_FRAME(state);
    TaggedValue tvalue = *POP(state);

    state->frame_count--;
    state->current = frame->start - frame->function->arity;

    PUSH(state, tvalue);
}


static void print_instruction()
{

}


void wdy_run(WdyState * state)
{
    initialize_stack(state, 20);
    allocate_call_frames(state);
    /* Call the 'main' function. */
    call(state, state->functions);

    while (*CURRENT_FRAME(state)->ip != OP_END)
    {
        uint32_t instruction = *CURRENT_FRAME(state)->ip++;
        LOG("Instruction %s\n", woody_opcodes[instruction]);

        switch (instruction)
        {
            case OP_LOAD_CONSTANT:
            {
                uint32_t index = *CURRENT_FRAME(state)->ip++;

                load_constants(state, index);
            } break;
            case OP_LOAD:
            {
                CallFrame * frame = CURRENT_FRAME(state);
                int32_t variable_index = *frame->ip++;
                TaggedValue * local = (TaggedValue *)(frame->start + variable_index);

                PUSH(state, *local);
            } break;
            case OP_STORE:
            {
                CallFrame * frame = CURRENT_FRAME(state);
                int32_t variable_index = *frame->ip++;
                TaggedValue * tvalue = POP(state);

                TaggedValue * local = (TaggedValue *)(frame->start + variable_index);

                local->value = tvalue->value;
                local->type = tvalue->type;
            } break;
            case OP_PLUS:
            case OP_MINUS:
            case OP_MULT:
            case OP_DIV:
            {
                do_arithmetic(state, instruction);
            } break;
            case OP_CALL:
            {
                TaggedValue * function = POP(state);
                call(state, function->value.function);
            } break;
            case OP_RETURN:
            {
                wdy_return(state);
            } break;
        }

        print_stack(state);
        check_stack(state);
    }
}
