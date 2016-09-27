#ifndef WOODY_OPCODES_H
#define WOODY_OPCODES_H


typedef enum
{
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_LOAD_CONSTANT,
    OP_LOAD_TRUE,
    OP_LOAD_FALSE,
    OP_LOAD,
    OP_STORE,
    OP_CALL,
    OP_RETURN,
    OP_END,
} Opcode;


extern const char * woody_opcodes[];


#endif
