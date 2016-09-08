#include "stdio.h"

#include "woody_opcodes.h"
#include "woody_vm.h"


#define POP(state) *((state)->stack_ptr); (state)->stack_ptr--
#define PUSH(state, value) ValueBufferPush(state->stack, value); (state)->stack_ptr++


void WoodyRun (WoodyState * state)
{
    printf("\nConstants:");

    for (uint32_t i = 0; i < state->constants->count; i++)
    {
        printf(" %f", state->constants->values[i]);
    }
    printf("\n\n");

    while (state->ip)
    {
        printf("\nStack:");

        for (uint32_t i = 0; i < state->stack->count; i++)
        {
            printf(" %f", state->stack->values[i]);
        }
        printf("\n\n");

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

                PUSH(state, a - b);
            } break;
            case OP_MULT:
            {
                double a = POP(state);
                double b = POP(state);

                PUSH(state, a * b);
            } break;
            case OP_DIV:
            {
                double a = POP(state);
                double b = POP(state);

                PUSH(state, a / b);
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
    }
}
