#include "stdio.h"

#include "woody_common.h"
#include "woody_opcodes.h"
#include "woody_memory.h"
#include "woody_value.h"
#include "woody_vm.h"


#define POP(state) --(state)->current
#define PUSH(state, value) *(state)->current++ = (value)


#define PUSH_NUMBER(state, number_value)             \
    state->current->value.number = (number_value); \
    state->current->type = WOODY_NUMBER;           \
    ++(state)->current

// void PrintInstructions(InstructionBuffer * buffer);

#define PrintBuffer(name, buffer, length)                        \
    printf("%s", #name);                                         \
    for (uint32_t i = 0; i < (buffer)->count; i++)               \
    {                                                            \
        switch ((buffer)->values[i].type)                        \
        {                                                        \
            case WOODY_NUMBER:                                   \
            {                                                    \
                printf("number: %d", (buffer)->values[i].value); \
            } break;                                             \
            case WOODY_BOOLEAN:                                  \
            {                                                    \
                printf("boolean: %i", );                         \
            } break;                                             \
            case WOODY_FUNCTION:                                 \
            {                                                    \
                printf("function: %i", (buffer->values[i]));     \
            } break;                                             \
            default:                                             \
            {                                                    \
                                                                 \
            } break;                                             \
        }                                                        \
    }                                                            \
    printf("\n\n")


static void PrintStack (WoodyState * state)
{
    printf("Stack: ");
    for (StackPtr i = state->stack; i <= state->current; i++)
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
}

static void DoArithmetic(WoodyState * state, Instruction i)
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
            result = a->value.number + b->value.number;
        } break;
        case OP_MINUS:
        {
            result = b->value.number - a->value.number;
        } break;
        case OP_MULT:
        {
            result = a->value.number * b->value.number;
        } break;
        case OP_DIV:
        {
            result = b->value.number / a->value.number;
        } break;
        default:
        {
            Assert(false, "Illegal arithmetic operation\n");
        } break;
    }

    PUSH_NUMBER(state, result);
}


void WoodyRun (WoodyState * state)
{
    // PrintBuffer(Constants, state->function->constants->values, state->function->constants->count);

    state->ip = state->function->code->values;

    while (*state->ip != OP_END)
    {
        uint32_t instruction = *(state->ip++);
        printf("Instruction %s\n", woody_opcodes[instruction]);

        switch (instruction)
        {
            case OP_CONSTANT:
            {
                uint32_t index = *(state->ip++);

                PUSH(state, state->function->constants->values[index]);
            } break;
            case OP_LOAD:
            {
                state->ip++;
            } break;
            case OP_STORE:
            {
                state->ip++;
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
        }

        PrintStack(state);
    }
}
