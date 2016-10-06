#include "woody_opcodes.h"


const char * woody_opcodes[] = {
    #define OP(name) #name,
    #include "woody_opcodes.def"
};
