#ifndef WOODY_OPCODES_H
#define WOODY_OPCODES_H


typedef enum
{
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_CONSTANT,
    OP_LOAD,
    OP_STORE,
} Opcode;


extern const char * woody_opcodes[];


#endif
