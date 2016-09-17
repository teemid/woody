#include "woody_memory.h"
#include "woody_value.h"


extern const char * woody_types[] = {
    "number",
    "boolean",
    "function",
};


DEFINE_BUFFER(Value, TaggedValue);
