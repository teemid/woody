#include <stdio.h>
#include <string.h>

#include "woody_common.h"
#include "woody_memory.h"
#include "woody_opcodes.h"
#include "woody_parser.h"
#include "woody_utils.h"

#define UNUSED(ptr) (void)(ptr)


DECLARE_TABLE(Symbol, char *, uint32_t);

DEFINE_TABLE(Symbol, char *, uint32_t);

#define HashString(key, length) djb2(key, length)


typedef struct
{
    WoodyFunction * function;
    SymbolTable * symbols;
} Prototype;


typedef struct
{
    WoodyState * state;
    WoodyLexer * lexer;
    Prototype * prototypes;
    Prototype * current;
    uint32_t prototype_count;
    uint32_t prototype_capacity;
} WoodyParser;


#define CompilingFunc(parser) (parser)->current;


static void NewPrototype (WoodyParser * parser)
{
    // Check to see if we need to increase the capacity of the prototype buffer.
    if (parser->prototype_count == parser->prototype_capacity)
    {
        parser->prototypes = ResizeBuffer(Prototype, parser->prototypes, parser->prototype_capacity * 2);
    }

    uint32_t initial_symbol_count = 20;

    WoodyFunction * parent = parser->current->function;

    Prototype * prototype = parser->prototypes + parser->prototype_count++;
    prototype->symbols = SymbolTableNew(initial_symbol_count);
    prototype->function = WoodyFunctionNew(parent);
    prototype->function->parent = parent;
}


WoodyParser * WoodyParserNew (WoodyState * state, WoodyLexer * lexer)
{
    WoodyParser * parser = (WoodyParser *)Allocate(sizeof(WoodyParser));
    parser->lexer = lexer;
    parser->state = state;

    uint32_t initial_prototype_count = 10;
    parser->prototypes = Buffer(Prototype, initial_prototype_count);
    parser->prototype_count = 0;
    parser->prototype_capacity = initial_prototype_count;

    // TODO: Initialize 'main' function.
    parser->current = parser->prototypes + parser->prototype_count++;
    parser->current->function = WoodyFunctionNew(NULL);
    parser->current->symbols = SymbolTableNew(20);

    // Save the function in state.
    parser->state->functions = parser->current->function;

    return parser;
}


typedef enum
{
    PRECEDENCE_NONE,
    PRECEDENCE_LOWEST,      // Sentinel
    PRECEDENCE_ASSIGNMENT,  // =
    PRECEDENCE_LOGICAL_OR,  // ||
    PRECEDENCE_LOGICAL_AND, // &&
    PRECEDENCE_EQUALITY,    // == !=
    PRECEDENCE_COMPARISON,  // < > <= >=
    PRECEDENCE_TERM,        // + -
    PRECEDENCE_FACTOR,      // * /
    PRECEDENCE_UNARY,       // - !
    PRECEDENCE_CALL,        // . () [] {}
} Precedence;


typedef void (* GrammarFn) (WoodyParser * parser);


typedef struct
{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
    char * name;
} GrammarRule;


static void ParsePrecedence   (WoodyParser * parser, Precedence precedence);
static void UnaryOperator     (WoodyParser * parser);
static void Expression        (WoodyParser * parser);
static void InfixOperator     (WoodyParser * parser);
static void VarStatement      (WoodyParser * parser);
static void FunctionStatement (WoodyParser * parser);
static void ReturnStatement   (WoodyParser * parser);
static void Identifier        (WoodyParser * parser);
static void OpenParen         (WoodyParser * parser);
static void Literal           (WoodyParser * parser);


#define NO_RULE                          { NULL,          NULL,          PRECEDENCE_NONE, NULL }
#define PREFIX(fn)                       { fn,            NULL,          PRECEDENCE_NONE, NULL }
#define INFIX(precedence, fn)            { NULL,          fn,            precedence,      NULL }
#define PREFIX_OPERATOR(name)            { UnaryOperator, NULL,          PRECEDENCE_NONE, name }
#define INFIX_OPERATOR(name, precedence) { NULL,          InfixOperator, precedence,      name }
#define OPERATOR(name)                   { UnaryOperator, InfixOperator, PRECEDENCE_TERM, name }


// NOTE (Emil): Each rule is associated with a token.
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

#define CurrentPrototype(parser) ((parser)->prototypes + (parser)->prototype_count - 1)
#define Constants(parser) (CurrentPrototype(parser)->function->constants)

#define PushOp(parser, op) InstructionBufferPush(CurrentPrototype(parser)->function->code, op)
#define PushOpArg(parser, op, argument) PushOp(parser, op); PushOp(parser, argument)

#define PrintToken(parser)                 \
    printf(                                \
        "%.*s %s\n",                       \
        Current(parser).length,            \
        Current(parser).start,             \
        woody_tokens[Current(parser).type] \
    )


static bool Match (WoodyParser * parser, WoodyTokenType match)
{
    if (Peek(parser) == match)
    {
        Next(parser);
        return true;
    }

    return false;
}


static void Expect(WoodyParser * parser, WoodyTokenType expected)
{
    if (!Match(parser, expected))
    {
        printf("Expected %s but got %s", woody_tokens[expected], woody_tokens[Current(parser).type]);
        exit(1);
    }
}


static void IgnoreNewLines (WoodyParser * parser)
{
    while (Match(parser, TOKEN_NEWLINE))
    {
        Next(parser);
    }
}


static uint32_t AddLocalVariable (WoodyParser * parser)
{
    Prototype * prototype = CurrentPrototype(parser);

    uint32_t local = prototype->symbols->count;
    uint32_t length = Current(parser).length;

    char * key = (char *)Allocate(length);
    memcpy(key, Current(parser).start, length * sizeof(char));

    uint32_t hash = HashString(key, length);

    SymbolTableAdd(prototype->symbols, key, hash, local);

    return local;
}


static uint32_t AddConstant (WoodyParser * parser)
{
    if (!Constants(parser))
    {
        WoodyFunctionInitializeConstants(CurrentPrototype(parser)->function);
    }

    TaggedValue tvalue;

    switch (Current(parser).type)
    {
        case TOKEN_NUMBER:
        {
            tvalue.value.number = Current(parser).value.number;
            tvalue.type = WOODY_NUMBER;
        } break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        {
            tvalue.value.boolean = Current(parser).value.boolean;
            tvalue.type = WOODY_BOOLEAN;
        } break;
        default:
        {
            printf("Illegal constant value.");
            exit(1);
        } break;
    }

    ValueBufferPush(Constants(parser), tvalue);

    return Constants(parser)->count - 1;
}


static void Statement (WoodyParser * parser)
{
    GrammarFn prefix = rules[Current(parser).type].prefix;

    if (!prefix)
    {
        printf("Expected prefix!\n");
        exit(1);
    }

    prefix(parser);
}


static void OpenParen (WoodyParser * parser)
{
    PrintToken(parser);

    Expression(parser);

    Expect(parser, TOKEN_CLOSE_PAREN);

    PrintToken(parser);
}


static void Expression (WoodyParser * parser)
{
    ParsePrecedence(parser, PRECEDENCE_LOWEST);
}


static void VarStatement (WoodyParser * parser)
{
    PrintToken(parser); // Print var

    Expect(parser, TOKEN_IDENTIFIER);

    PrintToken(parser); // Print identifier

    uint32_t local = AddLocalVariable(parser);

    if (Match(parser, TOKEN_EQ))
    {
        PrintToken(parser);

        Expression(parser);

        PushOpArg(parser, OP_STORE, local);
    }
}


static void ParseFunctionParameters (WoodyParser * parser)
{
    while (Match(parser, TOKEN_COMMA))
    {

    }
}


static void FunctionStatement (WoodyParser * parser)
{
    PrintToken(parser);

    Expect(parser, TOKEN_IDENTIFIER);
}


static void ReturnStatement (WoodyParser * parser)
{
    UNUSED(parser);
}


static void UnaryOperator (WoodyParser * parser)
{
    UNUSED(parser);
}


static void InfixOperator (WoodyParser * parser)
{
    PrintToken(parser); // Print the operator.

    uint32_t type = Current(parser).type;
    GrammarRule rule = rules[type];

    ParsePrecedence(parser, rule.precedence);

    uint32_t op = OP_PLUS + (type - TOKEN_PLUS);

    PushOp(parser, op);
}


static void Identifier (WoodyParser * parser)
{
    PrintToken(parser); // Print the identifier token.
}


static void Literal (WoodyParser * parser)
{
    PrintToken(parser);

    uint32_t constant = AddConstant(parser);

    PushOpArg(parser, OP_CONSTANT, constant);
}


static void ParsePrecedence (WoodyParser * parser, Precedence precedence)
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
    WoodyParser * parser = WoodyParserNew(state, lexer);

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

    // TODO (Emil): Clean up the parser.
}
