#include "woody_opcodes.h"


const char * woody_opcodes[] = {
    #define STR(s) #s
    #define OP(name) STR(OP_##name),
    #include "woody_opcodes.def"
};
