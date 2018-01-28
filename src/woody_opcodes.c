#include "woody_opcodes.h"


const char * woody_opcodes[] = {
    #define WDY_OP(name) #name,
    #include "woody_opcodes.def"
};
