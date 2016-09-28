#include <stdio.h>
#include <string.h>

#include "woody_common.h"
#include "woody_function.h"
#include "woody_lexer.h"
#include "woody_memory.h"
#include "woody_opcodes.h"
#include "woody_parser.h"
#include "woody_state.h"
#include "woody_utils.h"
#include "woody_value.h"

#define UNUSED(ptr) (void)(ptr)

#define HashString(key, length) djb2(key, length)


typedef struct
{
    uint32_t is_upvalue;
    uint32_t slot;
} Local;


DECLARE_TABLE(Symbol, const char *, uint32_t);

DEFINE_TABLE(Symbol, const char *, uint32_t);


typedef struct
{
    WoodyFunction * function;
    SymbolTable * symbols;
    uint32_t local_variables;
} Prototype;


typedef struct
{
    WoodyState * state;
    WoodyLexer * lexer;
    Prototype * prototypes;
    Prototype * current;
    uint32_t prototype_count;
    uint32_t prototype_capacity;
} Parser;


#define CurrentPrototype(parser) (parser)->current


static void EnsurePrototypeBuffer (Parser * parser)
{
    UNUSED(parser);
}


static Prototype * NewPrototype (Parser * parser, WoodyFunction * function)
{
    /* Check to see if we need to increase the capacity of the prototype buffer. */
    if (parser->prototype_count == parser->prototype_capacity)
    {
        parser->prototypes = ResizeBuffer(Prototype, parser->prototypes, parser->prototype_capacity + 1);
        parser->prototype_capacity += 1;
    }

    uint32_t initial_symbol_count = 20;

    Prototype * prototype = parser->prototypes + parser->prototype_count++;
    prototype->symbols = SymbolTableNew(initial_symbol_count);
    prototype->local_variables = 0;
    prototype->function = function;

    return prototype;
}


static Parser * NewParser (WoodyState * state, WoodyLexer * lexer)
{
    Parser * parser = (Parser *)Allocate(sizeof(Parser));
    parser->lexer = lexer;
    parser->state = state;

    uint32_t initial_prototype_count = 10;
    parser->prototypes = Buffer(Prototype, initial_prototype_count);
    parser->prototype_count = 0;
    parser->prototype_capacity = initial_prototype_count;

    /* TODO: Initialize 'main' function. */
    parser->current = parser->prototypes + parser->prototype_count++;
    parser->current->function = WoodyFunctionNew(state, NULL);
    parser->current->symbols = SymbolTableNew(20);

    /* Save the function in state. */
    parser->state->functions = parser->current->function;

    return parser;
}

static void FreeParser (Parser * parser)
{
    UNUSED(parser);
}


typedef enum
{
    PRECEDENCE_NONE,
    PRECEDENCE_LOWEST,      /* Sentinel */
    PRECEDENCE_ASSIGNMENT,  /* = */
    PRECEDENCE_LOGICAL_OR,  /* || */
    PRECEDENCE_LOGICAL_AND, /* && */
    PRECEDENCE_EQUALITY,    /* == != */
    PRECEDENCE_COMPARISON,  /* < > <= >= */
    PRECEDENCE_TERM,        /* + - */
    PRECEDENCE_FACTOR,      /* * / */
    PRECEDENCE_UNARY,       /* - ! */
    PRECEDENCE_CALL,        /* . () [] {} */
} Precedence;


typedef void (* GrammarFn) (Parser * parser);


typedef struct
{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
    char * name;
} GrammarRule;


static void ParsePrecedence   (Parser * parser, Precedence precedence);
static void UnaryOperator     (Parser * parser);
static void Expression        (Parser * parser);
static void InfixOperator     (Parser * parser);
static void VarStatement      (Parser * parser);
static void FunctionStatement (Parser * parser);
static void ReturnStatement   (Parser * parser);
static void Identifier        (Parser * parser);
static void OpenParen         (Parser * parser);
static void Literal           (Parser * parser);


#define NO_RULE                          { NULL,          NULL,          PRECEDENCE_NONE, NULL }
#define PREFIX(fn)                       { fn,            NULL,          PRECEDENCE_NONE, NULL }
#define INFIX(precedence, fn)            { NULL,          fn,            precedence,      NULL }
#define PREFIX_OPERATOR(name)            { UnaryOperator, NULL,          PRECEDENCE_NONE, name }
#define INFIX_OPERATOR(name, precedence) { NULL,          InfixOperator, precedence,      name }
#define OPERATOR(name)                   { UnaryOperator, InfixOperator, PRECEDENCE_TERM, name }


/* NOTE (Emil): Each rule is associated with a token. */
GrammarRule rules[] = {
    /* TOKEN_VAR         */ PREFIX(VarStatement),
    /* TOKEN_FUNCTION    */ PREFIX(FunctionStatement),
    /* TOKEN_RETURN      */ PREFIX(ReturnStatement),
    /* TOKEN_END         */ NO_RULE,
    /* TOKEN_TRUE        */ PREFIX(Literal),
    /* TOKEN_FALSE       */ PREFIX(Literal),
    /* TOKEN_COMMA       */ NO_RULE,
    /* TOKEN_PLUS        */ INFIX_OPERATOR("+", PRECEDENCE_TERM),
    /* TOKEN_MINUS       */ OPERATOR("-"),
    /* TOKEN_ASTERIX     */ INFIX_OPERATOR("*", PRECEDENCE_FACTOR),
    /* TOKEN_SLASH       */ INFIX_OPERATOR("/", PRECEDENCE_FACTOR),
    /* TOKEN_EQ          */ INFIX_OPERATOR("=", PRECEDENCE_ASSIGNMENT),
    /* TOKEN_OPEN_PAREN  */ PREFIX(OpenParen),
    /* TOKEN_CLOSE_PAREN */ NO_RULE,
    /* TOKEN_NUMBER      */ PREFIX(Literal),
    /* TOKEN_IDENTIFIER  */ PREFIX(Identifier),
    /* TOKEN_NEWLINE     */ NO_RULE,
    /* TOKEN_EOF         */ NO_RULE,
};


#define Next(parser) WoodyLexerNext(parser->lexer)
#define Peek(parser) WoodyLexerPeek(parser->lexer)

#define Current(parser) (parser)->lexer->current
#define Lookahead(parser) (parser)->lexer->lookahead

#define CurrentPrototype(parser) (parser)->current
#define Constants(parser) (CurrentPrototype(parser)->function->constants)

#define PushOp(parser, op) InstructionBufferPush(CurrentPrototype(parser)->function->code, op)
#define PushOpArg(parser, op, argument) PushOp(parser, op); PushOp(parser, argument)

#define PrintToken(parser)                  \
    printf(                                 \
        "%s %.*s\n",                        \
        woody_tokens[Current(parser).type], \
        (int)Current(parser).length,        \
        Current(parser).start               \
    )


static bool Match (Parser * parser, WoodyTokenType match)
{
    if (Peek(parser) == match)
    {
        Next(parser);
        return true;
    }

    return false;
}


static void Expect(Parser * parser, WoodyTokenType expected)
{
    if (!Match(parser, expected))
    {
        printf("Expected %s but got %s", woody_tokens[expected], woody_tokens[Current(parser).type]);
        exit(1);
    }
}


static void IgnoreNewLines (Parser * parser)
{
    while (Match(parser, TOKEN_NEWLINE))
    {
        Next(parser);
    }
}


static uint32_t AddLocalVariable (Parser * parser)
{
    Prototype * prototype = CurrentPrototype(parser);

    uint32_t local = prototype->local_variables++;
    uint32_t length = Current(parser).length;

    char * key = (char *)Allocate(length);
    memcpy(key, Current(parser).start, length * sizeof(char));

    uint32_t hash = HashString(key, length);

    SymbolTableAdd(prototype->symbols, key, hash, local);

    return local;
}


static uint32_t AddConstant (Parser * parser, TaggedValue tvalue)
{
    ValueBufferPush(Constants(parser), tvalue);

    return Constants(parser)->count - 1;
}


static uint32_t AddFunction (Parser * parser)
{
    WoodyFunction * parent = CurrentPrototype(parser)->function;
    WoodyFunction * function = WoodyFunctionNew(parser->state, parent);

    if (!parent->functions)
    {
        function->functions = (WoodyFunction **)Allocate(sizeof(WoodyFunction *));
        function->function_count = 0;
        function->function_capacity = 1;
    }
    else if (function->function_count == function->function_capacity)
    {
        size_t new_size = sizeof(WoodyFunction *) * function->function_capacity + 1;
        function->functions = Reallocate(function->functions, new_size);
        function->function_capacity += 1;
    }

    TaggedValue function_value = MakeFunction(function);

    uint32_t local = AddConstant(parser, function_value);

    /* NOTE (Emil): Add the symbol to the prototype so we can look it up later. */
    uint32_t bytes = Current(parser).length;
    char * key = (char *)Allocate(bytes);
    memcpy((void *)key, (void *)Current(parser).start, bytes);

    uint32_t hash = HashString(key, Current(parser).length);

    SymbolTableAdd(CurrentPrototype(parser)->symbols, key, hash, local);

    return local;
}


static void OpenParen (Parser * parser)
{
    PrintToken(parser);

    Expression(parser);

    Expect(parser, TOKEN_CLOSE_PAREN);

    PrintToken(parser);
}


static void Expression (Parser * parser)
{
    ParsePrecedence(parser, PRECEDENCE_LOWEST);
}


static void VarStatement (Parser * parser)
{
    PrintToken(parser); /* Print var */

    Expect(parser, TOKEN_IDENTIFIER);

    PrintToken(parser); /* Print identifier */

    uint32_t local = AddLocalVariable(parser);

    if (Match(parser, TOKEN_EQ))
    {
        PrintToken(parser);

        Expression(parser);

        PushOpArg(parser, OP_STORE, local);
    }
}


static Prototype * CreateFunction (Parser * parser)
{
    WoodyFunction * parent = CurrentPrototype(parser)->function;
    WoodyFunction * new = WoodyFunctionNew(parser->state, parent);

    Prototype * prototype = NewPrototype(parser, new);

    return prototype;
}


static void FunctionStatement (Parser * parser)
{
    PrintToken(parser);

    Expect(parser, TOKEN_IDENTIFIER);

    /* Create a function and prototype for the parser. */
    Prototype * prototype = CreateFunction(parser);
    uint32_t local = AddLocalVariable(parser);
    uint32_t constant = AddConstant(parser, MakeFunction(prototype->function));

    UNUSED(local);
    UNUSED(constant);

    Expect(parser, TOKEN_OPEN_PAREN);

    /* Parse the function body. */

    /* The function body is closed out with the end keyword. */
    Expect(parser, TOKEN_END);
}


static void ReturnStatement (Parser * parser)
{
    UNUSED(parser);
}


static void UnaryOperator (Parser * parser)
{
    UNUSED(parser);
}


static void InfixOperator (Parser * parser)
{
    PrintToken(parser); /* Print the operator. */

    uint32_t type = Current(parser).type;
    GrammarRule rule = rules[type];

    ParsePrecedence(parser, rule.precedence);

    uint32_t op = OP_PLUS + (type - TOKEN_PLUS);

    PushOp(parser, op);
}


static void Identifier (Parser * parser)
{
    PrintToken(parser); /* Print the identifier token. */

    if (Match(parser, TOKEN_OPEN_PAREN))
    {
        /* We have a function call. */
    }

    uint32_t hash = HashString(Current(parser).start, Current(parser).length);

    uint32_t local = SymbolTableFind(CurrentPrototype(parser)->symbols, hash)->value;

    PushOpArg(parser, OP_LOAD, local);
}


static void Literal (Parser * parser)
{
    PrintToken(parser);

    TaggedValue tvalue = MakeNumber(Current(parser).value.number);

    uint32_t constant = AddConstant(parser, tvalue);

    PushOpArg(parser, OP_LOAD_CONSTANT, constant);
}


static void ParsePrecedence (Parser * parser, Precedence precedence)
{
    GrammarFn prefix = rules[Next(parser)].prefix;

    if (!prefix)
    {
        printf("Expected prefix at the start of an expression.");
        exit(1);
    }

    prefix(parser);

    while (precedence < rules[Peek(parser)].precedence)
    {
        GrammarFn infix = rules[Next(parser)].infix;

        if (!infix)
        {
            printf("Expected an infix operator.");
            exit(1);
        }

        infix(parser);
    }
}


void WoodyParse (WoodyState * state, WoodyLexer * lexer)
{
    Parser * parser = NewParser(state, lexer);

    IgnoreNewLines(parser);

    while (!Match(parser, TOKEN_EOF))
    {
        ParsePrecedence(parser, PRECEDENCE_NONE);

        if (!Match(parser, TOKEN_NEWLINE))
        {
            Expect(parser, TOKEN_NEWLINE);
        }
    }

    PrintToken(parser);

    PushOp(parser, OP_END);

    FreeParser(parser);
}
