#ifndef WOODY_OPCODES_H
#define WOODY_OPCODES_H

typedef enum
{
    OP_CONSTANT,
    OP_LOAD,
    OP_STORE,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
} Opcode;

extern const char * woody_opcodes[];

#endif
