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


#define HASH_STRING(key, length) djb2(key, length)


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


DECLARE_TABLE(Symbol, symbol, const char *, Variable);

DEFINE_TABLE(Symbol, symbol, const char *, Variable);


typedef struct _Prototype
{
    struct _Prototype * parent;
    WdyFunction * function;
    SymbolTable * symbols;
} Prototype;


typedef struct
{
    WdyState * state;
    WdyLexer * lexer;
    Prototype * current;
    Prototype * prototypes;
    uint32_t prototype_count;
    uint32_t prototype_capacity;
} Parser;


#define CURRENT_PROTOTYPE(parser) (parser)->current


static void ensure_prototype_buffer(Parser * parser)
{
    if (parser->prototype_count == parser->prototype_capacity)
    {
        uint32_t new_capacity = parser->prototype_capacity;
        parser->prototypes = wdy_resize_buffer(Prototype, parser->prototypes, new_capacity);
    }
}


static Prototype * wdy_prototype_new(Parser * parser, WdyFunction * function)
{
    /* Check to see if we need to increase the capacity of the prototype buffer. */
    ensure_prototype_buffer(parser);

    uint32_t initial_symbol_count = 20;

    Prototype * prototype = parser->prototypes + parser->prototype_count++;
    prototype->parent = CURRENT_PROTOTYPE(parser);
    prototype->symbols = symbol_table_new(initial_symbol_count);
    prototype->function = function;

    return prototype;
}


static Parser * wdy_parser_new(WdyState * state, WdyLexer * lexer)
{
    Parser * parser = (Parser *)wdy_allocate(sizeof(Parser));
    parser->lexer = lexer;
    parser->state = state;

    uint32_t initial_prototype_count = 10;
    parser->prototypes = wdy_allocate_buffer(Prototype, initial_prototype_count);
    parser->prototype_count = 0;
    parser->prototype_capacity = initial_prototype_count;

    /* TODO: Initialize 'main' function. */
    parser->current = parser->prototypes + parser->prototype_count++;
    parser->current->parent = NULL;
    parser->current->function = wdy_function_new(state, NULL);
    parser->current->symbols = symbol_table_new(20);

    /* Save the function in state. */
    parser->state->functions = parser->current->function;

    return parser;
}

#define PUSH_PROTOTYPE(parser) parser->current = parser->current + 1
#define POP_PROTOTYPE(parser) parser->current = parser->current - 1

static void wdy_parser_free(Parser * parser)
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


static void parse_precedence   (Parser * parser, Precedence precedence);
static void unary_operator     (Parser * parser);
static void expression         (Parser * parser);
static void infix_operator     (Parser * parser);
static void var_statement      (Parser * parser);
static void function_statement (Parser * parser);
static void return_statement   (Parser * parser);
static void identifier         (Parser * parser);
static void open_paren         (Parser * parser);
static void literal            (Parser * parser);
static void if_statement       (Parser * parser);
static void while_statement    (Parser * parser);
static void for_statement      (Parser * parser);


#define NO_RULE                          { NULL          , NULL          , PRECEDENCE_NONE, NULL }
#define PREFIX(fn)                       { fn            , NULL          , PRECEDENCE_NONE, NULL }
#define INFIX(precedence, fn)            { NULL          , fn            , precedence     , NULL }
#define PREFIX_OPERATOR(name)            { unary_operator, NULL          , PRECEDENCE_NONE, name }
#define INFIX_OPERATOR(name, precedence) { NULL          , infix_operator, precedence     , name }
#define OPERATOR(name)                   { unary_operator, infix_operator, PRECEDENCE_TERM, name }


/* NOTE (Emil): Each rule is associated with a token. */
GrammarRule rules[] = {
    /* TOKEN_VAR         */ PREFIX(var_statement),
    /* TOKEN_FUNCTION    */ PREFIX(function_statement),
    /* TOKEN_RETURN      */ PREFIX(return_statement),
    /* TOKEN_END         */ NO_RULE,
    /* TOKEN_TRUE        */ PREFIX(literal),
    /* TOKEN_FALSE       */ PREFIX(literal),
    /* TOKEN IF          */ PREFIX(if_statement),
    /* TOKEN ELSE        */ NO_RULE,
    /* TOKEN BREAK       */ NO_RULE,
    /* TOKEN CONTINUE    */ NO_RULE,
    /* TOKEN WHILE       */ PREFIX(while_statement),
    /* TOKEN FOR         */ PREFIX(for_statement),
    /* TOKEN IN          */ NO_RULE,
    /* TOKEN DO          */ NO_RULE,
    /* TOKEN_COMMA       */ NO_RULE,
    /* TOKEN_PLUS        */ INFIX_OPERATOR("+", PRECEDENCE_TERM),
    /* TOKEN_MINUS       */ OPERATOR("-"),
    /* TOKEN_ASTERIX     */ INFIX_OPERATOR("*", PRECEDENCE_FACTOR),
    /* TOKEN_SLASH       */ INFIX_OPERATOR("/", PRECEDENCE_FACTOR),
    /* TOKEN_EQ          */ INFIX_OPERATOR("=", PRECEDENCE_ASSIGNMENT),
    /* TOKEN_OPEN_PAREN  */ PREFIX(open_paren),
    /* TOKEN_CLOSE_PAREN */ NO_RULE,
    /* TOKEN_NUMBER      */ PREFIX(literal),
    /* TOKEN_IDENTIFIER  */ PREFIX(identifier),
    /* TOKEN_NEWLINE     */ NO_RULE,
    /* TOKEN_EOF         */ NO_RULE,
};


#define NEXT(parser) wdy_lexer_next(parser->lexer)
#define PEEK(parser) wdy_lexer_peek(parser->lexer)

#define CURRENT(parser) (parser)->lexer->current
#define LOOKAHEAD(parser) (parser)->lexer->lookahead

#define CURRENT_PROTOTYPE(parser) (parser)->current
#define CONSTANTS(parser) (CURRENT_PROTOTYPE(parser)->function->constants)

#define PUSH_OP(parser, op) instruction_buffer_push(CURRENT_PROTOTYPE(parser)->function->code, op)
#define PUSH_OP_ARG(parser, op, argument) PUSH_OP(parser, op); PUSH_OP(parser, argument)

#define PRINT_TOKEN(parser)                 \
    LOG("%s %.*s\n",                        \
        woody_tokens[CURRENT(parser).type], \
        (int)CURRENT(parser).length,        \
        CURRENT(parser).start               \
    )


static bool match(Parser * parser, WdyTokenType match)
{
    if (PEEK(parser) == match)
    {
        NEXT(parser);
        return true;
    }

    return false;
}


static void expect(Parser * parser, WdyTokenType expected)
{
    ASSERT(
        match(parser, expected),
        "Expected %s but got %s\n",
        woody_tokens[expected],
        woody_tokens[CURRENT(parser).type]
    );
}


static void ignore_new_lines(Parser * parser)
{
    while (match(parser, TOKEN_NEWLINE)) { }
}


static Variable find_variable(Parser * parser)
{
    Prototype * proto = CURRENT_PROTOTYPE(parser);
    uint32_t hash = HASH_STRING(CURRENT(parser).start, CURRENT(parser).length);

    // Check if local variable.
    SymbolNode * node = symbol_table_find(proto->symbols, hash);

    if (node->value.type != VAR_UNDEFINED)
    {
        return node->value;
    }

    proto = proto->parent;
    while (proto)
    {
        node = symbol_table_find(proto->symbols, hash);

        if (node->value.type == VAR_UNDEFINED)
        {
            proto = proto->parent;
        }
        else
        {
            return node->value;
        }
    }

    LOG("Variable not found.");

    Variable var = { VAR_UNDEFINED, 0 };

    return var;
}


static uint32_t add_local_variable (Parser * parser)
{
    Prototype * prototype = CURRENT_PROTOTYPE(parser);

    Variable var = { VAR_LOCAL, 0 };
    var.slot = prototype->function->local_variables++;

    uint32_t length = CURRENT(parser).length;
    char * key = wdy_allocate_buffer(char, length);
    wdy_copy(CURRENT(parser).start, key, length);

    uint32_t hash = HASH_STRING(key, length);

    symbol_table_add(prototype->symbols, key, hash, var);

    return var.slot;
}


static uint32_t add_constant(Parser * parser, TaggedValue tvalue)
{
    value_buffer_push(CONSTANTS(parser), tvalue);

    return CONSTANTS(parser)->count - 1;
}


static void open_paren(Parser * parser)
{
    PRINT_TOKEN(parser);

    expression(parser);

    expect(parser, TOKEN_CLOSE_PAREN);

    PRINT_TOKEN(parser);
}


static void expression(Parser * parser)
{
    parse_precedence(parser, PRECEDENCE_LOWEST);
}


static void var_statement(Parser * parser)
{
    PRINT_TOKEN(parser); /* Print var */

    expect(parser, TOKEN_IDENTIFIER);

    PRINT_TOKEN(parser); /* Print identifier */

    uint32_t local = add_local_variable(parser);

    if (match(parser, TOKEN_EQ))
    {
        PRINT_TOKEN(parser);

        expression(parser);

        PUSH_OP_ARG(parser, OP_STORE, local);
    }
}


static inline Prototype * create_function(Parser * parser)
{
    WdyFunction * parent = CURRENT_PROTOTYPE(parser)->function;
    WdyFunction * new = wdy_function_new(parser->state, parent);
    Prototype * prototype = wdy_prototype_new(parser, new);

    return prototype;
}


static void parse_function_arguments(Parser * parser)
{
    Prototype * proto = CURRENT_PROTOTYPE(parser);

    expect(parser, TOKEN_OPEN_PAREN);

    PRINT_TOKEN(parser);

    int32_t arg = -1;

    while (match(parser, TOKEN_IDENTIFIER))
    {
        PRINT_TOKEN(parser);
        // Add identifier as argument and local variable.
        Variable local = { VAR_LOCAL, 0 };
        local.slot = arg--;
        uint32_t length = CURRENT(parser).length;
        char * argname = wdy_allocate_buffer(char, length);
        wdy_copy(CURRENT(parser).start, argname, length);

        uint32_t hash = HASH_STRING(argname, length);

        symbol_table_add(proto->symbols, argname, hash, local);

        proto->function->arity++;

        match(parser, TOKEN_COMMA);
    }

    expect(parser, TOKEN_CLOSE_PAREN);

    PRINT_TOKEN(parser);
}


static void function_statement(Parser * parser)
{
    PRINT_TOKEN(parser); // Print function

    expect(parser, TOKEN_IDENTIFIER);

    PRINT_TOKEN(parser); // Print the identifier

    // Create a function and prototype for the parser.
    Prototype * prototype = create_function(parser);
    // Add a local variable to the current scope.
    uint32_t local = add_local_variable(parser);
    // Add a function reference so we have some where to refer to when
    // we load the function to a variable and if we want to assign it to
    // another variable later.
    uint32_t constant = add_constant(parser, MakeFunction(prototype->function));

    // Change to compiling the new function.
    PUSH_PROTOTYPE(parser);
    parse_function_arguments(parser);

    ignore_new_lines(parser);

    while (!match(parser, TOKEN_END))
    {
        parse_precedence(parser, PRECEDENCE_NONE);

        if (match(parser, TOKEN_NEWLINE))
        {
            ignore_new_lines(parser);
        }
    }

    /* The function body is closed out with the end keyword. */
    PRINT_TOKEN(parser);

    ASSERT(
        CURRENT(parser).type == TOKEN_END,
        "Expected TOKEN_END but got %s\n",
        woody_tokens[CURRENT(parser).type]
    );

    // Change back to the parent prototype.
    POP_PROTOTYPE(parser);

    PUSH_OP_ARG(parser, OP_LOAD_CONSTANT, constant);
    PUSH_OP_ARG(parser, OP_STORE, local);
}


static void return_statement(Parser * parser)
{
    PRINT_TOKEN(parser);

    expression(parser);

    PUSH_OP(parser, OP_RETURN);
}


static void if_statement(Parser * parser)
{
    PRINT_TOKEN(parser);

    expression(parser);
}


static void while_statement(Parser * parser)
{

}


static void for_statement(Parser * parser)
{

}


static void unary_operator(Parser * parser)
{
    UNUSED(parser);
}


static void infix_operator(Parser * parser)
{
    PRINT_TOKEN(parser);

    uint32_t type = CURRENT(parser).type;
    GrammarRule rule = rules[type];

    parse_precedence(parser, rule.precedence);

    uint32_t op = OP_PLUS + (type - TOKEN_PLUS);

    PUSH_OP(parser, op);
}


static void call(Parser * parser)
{
    PRINT_TOKEN(parser);

    do {
        expression(parser);
    } while (match(parser, TOKEN_COMMA));

    expect(parser, TOKEN_CLOSE_PAREN);

    PRINT_TOKEN(parser);
}


static void identifier(Parser * parser)
{
    PRINT_TOKEN(parser);

    Variable var = find_variable(parser);

    if (match(parser, TOKEN_OPEN_PAREN))
    {
        call(parser);

        PUSH_OP_ARG(parser, OP_LOAD, var.slot);
        PUSH_OP(parser, OP_CALL);
    }
    else
    {
        PUSH_OP_ARG(parser, OP_LOAD, var.slot);
    }
}


static void literal(Parser * parser)
{
    PRINT_TOKEN(parser);

    TaggedValue tvalue = MakeNumber(CURRENT(parser).value.number);

    uint32_t constant = add_constant(parser, tvalue);

    PUSH_OP_ARG(parser, OP_LOAD_CONSTANT, constant);
}


static void parse_precedence(Parser * parser, Precedence precedence)
{
    GrammarFn prefix = rules[NEXT(parser)].prefix;

    ASSERT(prefix, "Expected prefix at the start of an expression.");

    prefix(parser);

    while (precedence < rules[PEEK(parser)].precedence)
    {
        GrammarFn infix = rules[NEXT(parser)].infix;

        ASSERT(infix, "Expected an infix operator.");

        infix(parser);
    }
}


void wdy_parse(WdyState * state, WdyLexer * lexer)
{
    Parser * parser = wdy_parser_new(state, lexer);

    while (!match(parser, TOKEN_EOF))
    {
        ignore_new_lines(parser);

        parse_precedence(parser, PRECEDENCE_NONE);

        if (match(parser, TOKEN_NEWLINE))
        {
            ignore_new_lines(parser);
        }
    }

    PRINT_TOKEN(parser);

    PUSH_OP(parser, OP_END);

    wdy_parser_free(parser);
}
