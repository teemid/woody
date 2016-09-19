#include "stdio.h"

#include "woody_common.h"
#include "woody_opcodes.h"
#include "woody_memory.h"
#include "woody_value.h"
#include "woody_vm.h"


#define POP(state) --(state)->current
#define PUSH(state, value) *(state)->current++ = (value)

#define PUSH_NUMBER(state, number_value)           \
    state->current->value.number = (number_value); \
    state->current->type = WOODY_NUMBER;           \
    ++(state)->current

#define PUSH_UNDEFINED(state) \
    (state)->current->value.type = WOODY_UNDEFINED; \
    ++(state)->current

#define CurrentFrame(state) ((state)->frames + (state)->frame_count - 1)
#define Constants(state) CurrentFrame(state)->function->constants


static void PrintStack (WoodyState * state)
{
    printf("Stack: ");
    for (StackPtr i = state->stack; i < state->current; i++)
    {
        switch (i->type)
        {
            case WOODY_NUMBER:
            {
                printf("%f ", i->value.number);
            } break;
            case WOODY_BOOLEAN:
            {
                printf("%i ", i->value.boolean);
            } break;
            default:
            {
                printf("\nError illegal value.");
            } break;
        }
    }

    printf("\n");
}


static void CheckStack (WoodyState * state)
{
    if (state->stack == state->top)
    {

    }
}


static void LoadConstant (WoodyState * state, uint32_t index)
{
    TaggedValue value = Constants(state)->values[index];

    state->current->value = value.value;
    state->current->type = value.type;

    state->current++;
}


static void DoArithmetic (WoodyState * state, Instruction i)
{
    TaggedValue * a = POP(state);
    TaggedValue * b = POP(state);

    Assert(
        a->type != WOODY_NUMBER || b->type != WOODY_NUMBER,
        "Trying arithmetic on non-number types"
    );

    double result = 0;

    switch (i)
    {
        case OP_PLUS:
        {
            result = Number(a) + Number(b);
        } break;
        case OP_MINUS:
        {
            result = Number(b) - Number(a);
        } break;
        case OP_MULT:
        {
            result = Number(a) * Number(b);
        } break;
        case OP_DIV:
        {
            result = Number(b) / Number(a);
        } break;
        default:
        {
            Assert(false, "Illegal arithmetic operation\n");
        } break;
    }

    PUSH_NUMBER(state, result);
}


static void InitializeStack (WoodyState * state, uint32_t initial_stack_size)
{
    state->stack = Buffer(TaggedValue, initial_stack_size);
    state->current = state->stack;
    state->top = state->stack + initial_stack_size;

    state->current = state->stack;
}


static void AllocateCallFrames (WoodyState * state)
{
    uint32_t initial_frame_capacity = 5;
    state->frames = Buffer(CallFrame, initial_frame_capacity);
    state->frame_count = 0;
    state->frame_capacity = initial_frame_capacity;
}


static void Call (WoodyState * state, WoodyFunction * function)
{
    CallFrame * frame = state->frames + state->frame_count++;
    frame->function = function;
    frame->ip = function->code->values;
    frame->start = state->current;


}


void WoodyRun (WoodyState * state)
{
    InitializeStack(state, 20);
    AllocateCallFrames(state);
    // Call the 'main' function.
    Call(state, state->functions);

    while (*CurrentFrame(state)->ip != OP_END)
    {
        uint32_t instruction = *CurrentFrame(state)->ip++;
        printf("Instruction %s\n", woody_opcodes[instruction]);

        switch (instruction)
        {
            case OP_CONSTANT:
            {
                uint32_t index = *CurrentFrame(state)->ip++;

                LoadConstant(state, index);
            } break;
            case OP_LOAD:
            {
                CurrentFrame(state)->ip++;
            } break;
            case OP_STORE:
            {
                CurrentFrame(state)->ip++;
            } break;
            case OP_PLUS:
            case OP_MINUS:
            case OP_MULT:
            case OP_DIV:
            {
                DoArithmetic(state, instruction);
            } break;
            case OP_CALL:
            {

            } break;
            case OP_RETURN:
            {

            } break;
        }

        PrintStack(state);
    }
}
