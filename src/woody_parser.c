#include "stdio.h"

#include "woody_parser.h"
#include "woody_opcodes.h"
#include "woody_utils.h"


#define PrintToken(lexer)                   \
    printf(                                 \
        "%.*s %s\n",                        \
        (lexer)->current.length,            \
        (lexer)->current.start,             \
        woody_tokens[(lexer)->current.type] \
    )

#define UNUSED(ptr) (void)(ptr)


typedef struct
{
    WoodyState * state;
    WoodyLexer * lexer;
    SymbolTable * symbols;
} WoodyParser;


WoodyParser * WoodyParserNew (WoodyState * state, WoodyLexer * lexer)
{
    WoodyParser * parser = malloc(sizeof(WoodyParser));
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


static void ParsePrecedence (WoodyParser * parser, Precedence precedence);
static void UnaryOperator   (WoodyParser * parser);
static void Expression      (WoodyParser * parser);
static void InfixOperator   (WoodyParser * parser);
static void VarStatement    (WoodyParser * parser);
static void Identifier      (WoodyParser * parser);
static void OpenParen       (WoodyParser * parser);
static void Literal         (WoodyParser * parser);


#define NO_RULE                          { NULL,          NULL,          PRECEDENCE_NONE, NULL }
#define PREFIX(fn)                       { fn,            NULL,          PRECEDENCE_NONE, NULL }
#define INFIX(precedence, fn)            { NULL,          fn,            precedence,      NULL }
#define PREFIX_OPERATOR(name)            { UnaryOperator, NULL,          PRECEDENCE_NONE, name }
#define INFIX_OPERATOR(name, precedence) { NULL,          InfixOperator, precedence,      name }
#define OPERATOR(name)                   { UnaryOperator, InfixOperator, PRECEDENCE_TERM, name }


// NOTE (Emil): Each rule is associated with a token.
GrammarRule rules[] = {
    /* TOKEN_PLUS        */ INFIX_OPERATOR("+", PRECEDENCE_TERM),
    /* TOKEN_MINUS       */ OPERATOR("-"),
    /* TOKEN_ASTERIX     */ INFIX_OPERATOR("*", PRECEDENCE_FACTOR),
    /* TOKEN_SLASH       */ INFIX_OPERATOR("/", PRECEDENCE_FACTOR),
    /* TOKEN_VAR         */ PREFIX(VarStatement),
    /* TOKEN_EQ          */ INFIX_OPERATOR("=", PRECEDENCE_ASSIGNMENT),
    /* TOKEN_OPEN_PAREN  */ PREFIX(OpenParen),
    /* TOKEN_CLOSE_PAREN */ NO_RULE,
    /* TOKEN_NUMBER      */ PREFIX(Literal),
    /* TOKEN_IDENTIFIER  */ PREFIX(Identifier),
    /* TOKEN_EOF         */ NO_RULE,
};


static void Statement (WoodyParser * parser)
{
    GrammarFn prefix = rules[parser->lexer->current.type].prefix;

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
    PrintToken(parser->lexer);

    WoodyLexerNext(parser->lexer);

    Expression(parser);

    WoodyLexerNext(parser->lexer);

    PrintToken(parser->lexer);
}


static void Expression (WoodyParser * parser)
{
    ParsePrecedence(parser, PRECEDENCE_LOWEST);
}


static void VarStatement (WoodyParser * parser)
{
    WoodyLexer * lexer = parser->lexer;

    PrintToken(lexer);  // Print the Var token.

    if (lexer->current.type != TOKEN_VAR)
    {
        printf("Expected var statement!\n");
        exit(1);
    }

    if (WoodyLexerNext(lexer) != TOKEN_IDENTIFIER)
    {
        printf("Expected identifier!\n");
        exit(1);
    }

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
    if (WoodyLexerPeek(lexer) == TOKEN_EQ)
    {
        // Move to and print the equality token.
        WoodyLexerNext(lexer); PrintToken(lexer);

        // Move to token after TOKEN_EQ, which we assume is an expression.
        WoodyLexerNext(lexer);
        Expression(parser);
    }
}


static void UnaryOperator (WoodyParser * parser)
{
    UNUSED(parser);
}


static void InfixOperator (WoodyParser * parser)
{
    PrintToken(parser->lexer);

    uint32_t type = parser->lexer->current.type;
    GrammarRule rule = rules[type];

    WoodyLexerNext(parser->lexer);

    ParsePrecedence(parser, rule.precedence);

    InstructionBufferPush(parser->state->code, OP_PLUS + type);
}


static void Identifier (WoodyParser * parser)
{
    PrintToken(parser->lexer); // Print the identifier token.
}


static void Literal (WoodyParser * parser)
{
    PrintToken(parser->lexer);

    WoodyState * state = parser->state;
    WoodyLexer * lexer = parser->lexer;

    ValueBufferPush(state->constants, lexer->current.value);
    InstructionBufferPush(state->code, OP_CONSTANT);
    InstructionBufferPush(state->code, state->constants->count - 1);
}


static void ParsePrecedence (WoodyParser * parser, Precedence precedence)
{
    GrammarFn prefix = rules[parser->lexer->current.type].prefix;

    if (!prefix)
    {
        printf("Expected prefix at the start of an expression.");
        exit(1);
    }

    prefix(parser);

    while (precedence < rules[WoodyLexerPeek(parser->lexer)].precedence)
    {
        GrammarFn infix = rules[WoodyLexerNext(parser->lexer)].infix;

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

    while (WoodyLexerNext(parser->lexer) != TOKEN_EOF)
    {
        ParsePrecedence(parser, PRECEDENCE_NONE);
    }

    PrintToken(parser->lexer);

    InstructionBufferPush(state->code, OP_END);
}
