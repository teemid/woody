#include "stdio.h"
#include "stdbool.h"

#include "woody_memory.h"
#include "woody_opcodes.h"
#include "woody_parser.h"
#include "woody_utils.h"

#define UNUSED(ptr) (void)(ptr)


typedef struct WoodyPrototype
{
    uint8_t arity;
    struct WoodyPrototype * parent;
    struct WoodyPrototype * functions;
    uint32_t function_count;
    uint32_t function_capacity;
    SymbolTable * symbols;
} WoodyPrototype;


typedef struct
{
    WoodyState * state;
    WoodyLexer * lexer;
    SymbolTable * symbols;
} WoodyParser;


WoodyParser * WoodyParserNew (WoodyState * state, WoodyLexer * lexer)
{
    WoodyParser * parser = (WoodyParser *)Allocate(sizeof(WoodyParser));
    parser->lexer = lexer;
    parser->state = state;
    parser->symbols = SymbolTableNew(20);

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


static uint32_t AddLocalVariable (WoodyParser * parser)
{
    UNUSED(parser);

    return 0;
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
    PrintToken(parser);

    Expect(parser, TOKEN_IDENTIFIER);

    Identifier(parser);

    /*
     * NOTE (Emil):
     * Here we need a temp structure to hold identifier -> variable number.
     */
    uint32_t local = AddLocalVariable(parser);

    UNUSED(local);

    /*
     * NOTE (Emil):
     * Implement lookahead and check if the next token is '='.
     * If it is we go to assignment. If not return, the statement
     * declared a variable and is done.
     */
    if (Match(parser, TOKEN_EQ))
    {
        // Move to and print the equality token.
        PrintToken(parser);

        // Move to token after TOKEN_EQ, which we assume is an expression.
        // Next(parser);
        Expression(parser);

        InstructionBufferPush(parser->state->code, OP_STORE);
        InstructionBufferPush(parser->state->code, local);
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

    // Next(parser);

    ParsePrecedence(parser, rule.precedence);

    InstructionBufferPush(parser->state->code, OP_PLUS + type);
}


static void Identifier (WoodyParser * parser)
{
    PrintToken(parser); // Print the identifier token.
}


static void Literal (WoodyParser * parser)
{
    PrintToken(parser);

    WoodyState * state = parser->state;

    switch (Current(parser).type)
    {
        case TOKEN_NUMBER:
        {

        } break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        {
            TaggedValue tvalue;
            tvalue.value.number = Current(parser).value.number;
            tvalue.type = WOODY_NUMBER;

            ValueBufferPush(state->constants, tvalue);
        } break;
        default:
        {
            Assert(false, "Unexpected value.");
        }
    }


    InstructionBufferPush(state->code, OP_CONSTANT);
    InstructionBufferPush(state->code, state->constants->count - 1);
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

    InstructionBufferPush(state->code, OP_END);
}
