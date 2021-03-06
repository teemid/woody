#include "woody_memory.h"
#include "woody_value.h"


const char * woody_types[] = {
    "number",
    "boolean",
    "function",
};


DEFINE_BUFFER(Value, value, TaggedValue);


int32_t value_buffer_find(ValueBuffer * buffer, TaggedValue value)
{
    for (size_t i = 0; i < buffer->count; i++)
    {
        TaggedValue * search = buffer->values + i;

        if (search->type == value.type)
        {
            switch (search->type)
            {
                case TYPE_NUMBER:
                {
                    if (wdy_number(search) == wdy_number(&value))
                    {
                        return i;
                    }
                } break;
                case TYPE_FALSE:
                case TYPE_TRUE:
                {
                    return i;
                } break;
                case TYPE_FUNCTION:
                {
                    if (search->value.function == value.value.function)
                    {
                        return i;
                    }
                } break;
                default:
                {

                } break;
            }
        }
    }

    return -1;
}


TaggedValue wdy_make_number(double number)
{
    TaggedValue tvalue;
    tvalue.value.number = number;
    tvalue.type = TYPE_NUMBER;

    return tvalue;
}


TaggedValue wdy_make_function(WdyFunction * function)
{
    TaggedValue tvalue;
    tvalue.value.function = function;
    tvalue.type = TYPE_FUNCTION;

    return tvalue;
}
