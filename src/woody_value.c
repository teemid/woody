#include "woody_memory.h"
#include "woody_value.h"


extern const char * woody_types[] = {
    "number",
    "boolean",
    "function",
};


DEFINE_BUFFER(Value, TaggedValue);


int32_t ValueBufferFind(ValueBuffer * buffer, TaggedValue value)
{
    for (size_t i = 0; i < buffer->count; i++)
    {
        if (buffer->values[i].type == value.type)
        {
            switch (buffer->values[i].type)
            {
            }
            return i;
        }
    }

    return -1;
}
