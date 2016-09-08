#include "stdio.h"

#include "woody_opcodes.h"
#include "woody_vm.h"


#define POP(state) *(--(state)->stack_ptr)
#define PUSH(state, value) *(state)->stack_ptr++ = (value)

#define PrintBuffer(name, buffer) \
    printf("%s", #name); \
    for (uint32_t i = 0; i < (buffer)->count; i++) \
    { \
        printf(" %f", (buffer)->values[i]); \
    } \
    printf("\n\n")

#define PrintStack(state) \
    printf("Stack: "); \
    for (double * curr = state->stack; curr != (state)->stack_ptr; curr++) \
    { \
        printf(" %f", *curr); \
    } \
    printf("\n\n")


void WoodyRun (WoodyState * state)
{
    while (*state->ip != OP_END)
    {
        uint32_t instruction = *(state->ip++);
        printf("Instruction %s\n", woody_opcodes[instruction]);

        switch (instruction)
        {
            case OP_PLUS:
            {
                double a = POP(state);
                double b = POP(state);

                printf("%f + %f\n", a, b);

                PUSH(state, a + b);
            } break;
            case OP_MINUS:
            {
                double a = POP(state);
                double b = POP(state);

                printf("%f - %f\n", a, b);

                PUSH(state, a - b);
            } break;
            case OP_MULT:
            {
                double a = POP(state);
                double b = POP(state);

                printf("%f * %f\n", a, b);

                PUSH(state, a * b);
            } break;
            case OP_DIV:
            {
                double a = POP(state);
                double b = POP(state);

                printf("%f / %f\n", b, a);

                PUSH(state, b / a);
            } break;
            case OP_CONSTANT:
            {
                uint32_t index = *(state->ip++);

                PUSH(state, state->constants->values[index]);
            } break;
            case OP_LOAD:
            {

            } break;
            case OP_STORE:
            {

            } break;
        }

        PrintStack(state);
    }
}
