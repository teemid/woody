#ifndef WOODY_OPCODES_H
#define WOODY_OPCODES_H


typedef enum
{
    #define WDY_OP(name) OP_##name,
    #include "woody_opcodes.def"
} Opcode;


extern const char * woody_opcodes[];


#endif
