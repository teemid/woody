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


typedef enum
{
    VAR_UNDEFINED,
    VAR_LOCAL,
    VAR_UPVALUE,
} VariableType;


typedef struct
{
    VariableType type;
    int32_t slot;
} Variable;


DECLARE_TABLE(Symbol, const char *, Variable);

DEFINE_TABLE(Symbol, const char *, Variable);


typedef struct _Prototype
{
    struct _Prototype * parent;
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
} Parser;


#define CurrentPrototype(parser) (parser)->current


static void EnsurePrototypeBuffer (Parser * parser)
{
    if (parser->prototype_count == parser->prototype_capacity)
    {
        uint32_t new_capacity = parser->prototype_capacity;
        parser->prototypes = ResizeBuffer(Prototype, parser->prototypes, new_capacity);
    }
}


static Prototype * NewPrototype (Parser * parser, WoodyFunction * function)
{
    /* Check to see if we need to increase the capacity of the prototype buffer. */
    EnsurePrototypeBuffer(parser);

    uint32_t initial_symbol_count = 20;

    Prototype * prototype = parser->prototypes + parser->prototype_count++;
    prototype->parent = CurrentPrototype(parser);
    prototype->symbols = SymbolTableNew(initial_symbol_count);
    prototype->function = function;

    return prototype;
}


static Parser * NewParser (WoodyState * state, WoodyLexer * lexer)
{
    Parser * parser = (Parser *)Allocate(sizeof(Parser));
    parser->lexer = lexer;
    parser->state = state;

    uint32_t initial_prototype_count = 10;
    parser->prototypes = AllocateBuffer(Prototype, initial_prototype_count);
    parser->prototype_count = 0;
    parser->prototype_capacity = initial_prototype_count;

    /* TODO: Initialize 'main' function. */
    parser->current = parser->prototypes + parser->prototype_count++;
    parser->current->parent = NULL;
    parser->current->function = WoodyFunctionNew(state, NULL);
    parser->current->symbols = SymbolTableNew(20);

    /* Save the function in state. */
    parser->state->functions = parser->current->function;

    return parser;
}

#define PushPrototype(parser) parser->current = parser->current + 1
#define PopPrototype(parser) parser->current = parser->current - 1

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
static void IfStatement       (Parser * parser);
static void WhileStatement    (Parser * parser);
static void ForStatement      (Parser * parser);


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
    /* TOKEN IF          */ PREFIX(IfStatement),
    /* TOKEN ELSE        */ NO_RULE,
    /* TOKEN BREAK       */ NO_RULE,
    /* TOKEN CONTINUE    */ NO_RULE,
    /* TOKEN WHILE       */ PREFIX(WhileStatement),
    /* TOKEN FOR         */ PREFIX(ForStatement),
    /* TOKEN IN          */ NO_RULE,
    /* TOKEN DO          */ NO_RULE,
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
    Log("%s %.*s\n",                        \
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
    Assert(
        Match(parser, expected),
        "Expected %s but got %s\n",
        woody_tokens[expected],
        woody_tokens[Current(parser).type]
    );
}


static void IgnoreNewLines (Parser * parser)
{
    while (Match(parser, TOKEN_NEWLINE)) { }
}


static Variable FindVariable (Parser * parser)
{
    Prototype * proto = CurrentPrototype(parser);
    uint32_t hash = HashString(Current(parser).start, Current(parser).length);

    // Check if local variable.
    SymbolNode * node = SymbolTableFind(proto->symbols, hash);

    if (node->value.type != VAR_UNDEFINED)
    {
        return node->value;
    }

    proto = proto->parent;
    while (proto)
    {
        node = SymbolTableFind(proto->symbols, hash);

        if (node->value.type == VAR_UNDEFINED)
        {
            proto = proto->parent;
        }
        else
        {
            return node->value;
        }
    }

    Log("Variable not found.");

    Variable var = { VAR_UNDEFINED, 0 };

    return var;
}


static uint32_t AddLocalVariable (Parser * parser)
{
    Prototype * prototype = CurrentPrototype(parser);

    Variable var = { 0, -1 };
    var.slot = prototype->function->local_variables++;
    uint32_t length = Current(parser).length;

    char * key = AllocateBuffer(char, length);
    Copy(Current(parser).start, key, length);

    uint32_t hash = HashString(key, length);

    SymbolTableAdd(prototype->symbols, key, hash, var);

    return var.slot;
}


static uint32_t AddConstant (Parser * parser, TaggedValue tvalue)
{
    ValueBufferPush(Constants(parser), tvalue);

    return Constants(parser)->count - 1;
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


static void ParseFunctionArguments (Parser * parser)
{
    Prototype * proto = CurrentPrototype(parser);

    Expect(parser, TOKEN_OPEN_PAREN);

    PrintToken(parser);

    int32_t arg = -1;

    while (Match(parser, TOKEN_IDENTIFIER))
    {
        PrintToken(parser);
        // Add identifier as argument and local variable.
        Variable local = { VAR_LOCAL, 0 };
        local.slot = arg--;
        uint32_t length = Current(parser).length;
        char * argname = AllocateBuffer(char, length);
        Copy(Current(parser).start, argname, length);

        uint32_t hash = HashString(argname, length);

        SymbolTableAdd(proto->symbols, argname, hash, local);

        proto->function->arity++;

        Match(parser, TOKEN_COMMA);
    }

    Expect(parser, TOKEN_CLOSE_PAREN);

    PrintToken(parser);
}


static void FunctionStatement (Parser * parser)
{
    PrintToken(parser); // Print function

    Expect(parser, TOKEN_IDENTIFIER);

    PrintToken(parser); // Print the identifier

    /* Create a function and prototype for the parser. */
    Prototype * prototype = CreateFunction(parser);
    // Add a local variable to the current scope.
    uint32_t local = AddLocalVariable(parser);
    // Add a function reference so we have some where to refer to when
    // we load the function to a variable and if we want to assign it to
    // another variable later.
    uint32_t constant = AddConstant(parser, MakeFunction(prototype->function));

    // Change to compiling the new function.
    PushPrototype(parser);
    ParseFunctionArguments(parser);

    IgnoreNewLines(parser);

    while (!Match(parser, TOKEN_END))
    {
        ParsePrecedence(parser, PRECEDENCE_NONE);

        if (Match(parser, TOKEN_NEWLINE))
        {
            IgnoreNewLines(parser);
        }
    }

    /* The function body is closed out with the end keyword. */
    PrintToken(parser);

    Assert(
        Current(parser).type == TOKEN_END,
        "Expected TOKEN_END but got %s\n",
        woody_tokens[Current(parser).type]
    );

    // Change back to the parent prototype.
    PopPrototype(parser);

    PushOpArg(parser, OP_LOAD_CONSTANT, constant);
    PushOpArg(parser, OP_STORE, local);
}


static void ReturnStatement (Parser * parser)
{
    PrintToken(parser);

    Expression(parser);

    PushOp(parser, OP_RETURN);
}


static void IfStatement (Parser * parser)
{
    PrintToken(parser);

    Expression(parser);
}


static void WhileStatement (Parser * parser)
{

}


static void ForStatement (Parser * parser)
{

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


static void Call(Parser * parser)
{
    PrintToken(parser);

    do {
        Expression(parser);
    } while (Match(parser, TOKEN_COMMA));

    Expect(parser, TOKEN_CLOSE_PAREN);

    PrintToken(parser);
}


static void Identifier (Parser * parser)
{
    PrintToken(parser); /* Print the identifier token. */

    Variable var = FindVariable(parser);

    if (Match(parser, TOKEN_OPEN_PAREN))
    {
        Call(parser);

        PushOpArg(parser, OP_LOAD, var.slot);
        PushOp(parser, OP_CALL);
    }
    else
    {
        PushOpArg(parser, OP_LOAD, var.slot);
    }
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

    Assert(prefix, "Expected prefix at the start of an expression.");

    prefix(parser);

    while (precedence < rules[Peek(parser)].precedence)
    {
        GrammarFn infix = rules[Next(parser)].infix;

        Assert(infix, "Expected an infix operator.");

        infix(parser);
    }
}


void WoodyParse (WoodyState * state, WoodyLexer * lexer)
{
    Parser * parser = NewParser(state, lexer);

    while (!Match(parser, TOKEN_EOF))
    {
        IgnoreNewLines(parser);

        ParsePrecedence(parser, PRECEDENCE_NONE);

        if (Match(parser, TOKEN_NEWLINE))
        {
            IgnoreNewLines(parser);
        }
    }

    PrintToken(parser);

    PushOp(parser, OP_END);

    FreeParser(parser);
}
