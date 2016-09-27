#ifndef WOODY_VALUE_H
#define WOODY_VALUE_H

#include "woody_utils.h"


/* Forward declaration */
typedef struct WoodyFunction WoodyFunction;
typedef struct woody_string_t WoodyString;


typedef enum
{
    TYPE_NUMBER,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_FUNCTION,
} WoodyType;


extern const char * woody_types[];


typedef union
{
    double number;
    WoodyFunction * function;
} WoodyValue;


/* Object is a linked list of all heap allocated objects. */
typedef struct Object
{
    WoodyType type;
    struct Object * next;
} Object;


typedef struct TaggedValue
{
    WoodyValue value;
    WoodyType type;
} TaggedValue;


#define Type(tvalue) (tvalue)->type

#define IsNumber(tvalue) Type(tvalue) == TYPE_NUMBER
#define IsFunction(tvalue) Type(tvalue) == TYPE_FUNCTION
#define IsBoolean(tvalue) (Type(tvalue) == TYPE_FALSE) || (Type(tvalue) == TYPE_FALSE)

#define Number(tvalue) (tvalue)->value.number
#define Boolean(tvalue) (tvalue)->value.boolean
#define Function(tvalue) (tvalue)->value.function

#define MakeBoolean(value) (value ? TYPE_TRUE : TYPE_FALSE)
#define MakeNumber(value) (WoodyMakeNumber(value))
#define MakeFunction(function_pointer) (WoodyMakeFunction(function_pointer))


DECLARE_BUFFER(Value, TaggedValue);


int32_t ValueBufferFind (ValueBuffer * buffer, TaggedValue value);

TaggedValue WoodyMakeNumber (double number);
TaggedValue WoodyMakeFunction (WoodyFunction * function);


#endif
