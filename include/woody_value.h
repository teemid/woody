#ifndef WOODY_VALUE_H
#define WOODY_VALUE_H

#include "woody_utils.h"

typedef struct WoodyFunction WdyFunction;
typedef struct WoodyString WdyString;


typedef enum
{
    TYPE_NUMBER,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_FUNCTION,
} WdyType;


extern const char * woody_types[];


typedef union
{
    double number;
    WdyFunction * function;
} WdyValue;


/* Object is a linked list of all heap allocated objects. */
typedef struct Object
{
    WdyType type;
    struct Object * next;
} Object;


typedef struct TaggedValue
{
    WdyValue value;
    WdyType type;
} TaggedValue;


#define wdy_type(tvalue) (tvalue)->type

#define wdy_is_number(tvalue) wdy_type(tvalue) == TYPE_NUMBER
#define wdy_is_function(tvalue) wdy_type(tvalue) == TYPE_FUNCTION
#define wdy_is_boolean(tvalue) (wdy_type(tvalue) == TYPE_FALSE) || (wdy_type(tvalue) == TYPE_FALSE)

#define wdy_number(tvalue) (tvalue)->value.number
#define wdy_boolean(tvalue) (tvalue)->value.boolean
#define wdy_function(tvalue) (tvalue)->value.function

#define MakeBoolean(value) (value ? TYPE_TRUE : TYPE_FALSE)
#define MakeNumber(value) (wdy_make_number(value))
#define MakeFunction(function_pointer) (wdy_make_function(function_pointer))


DECLARE_BUFFER(Value, value, TaggedValue);


int32_t value_buffer_find(ValueBuffer * buffer, TaggedValue value);

TaggedValue wdy_make_number(double number);
TaggedValue wdy_make_function(WdyFunction * function);


#endif
