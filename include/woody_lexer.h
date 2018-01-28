#ifndef WOODY_LEXER_H
#define WOODY_LEXER_H


#include "woody_common.h"


typedef enum
{
    #define WDY_TOKEN(t) TOKEN_##t,
    #include "woody_tokens.def"
} WdyTokenType;


extern const char * woody_tokens[];


typedef union {
    double number;
    uint32_t boolean;
} Value;


typedef struct
{
    WdyTokenType type;
    Value value;
    const char * start;
    size_t length;
} WdyToken;


typedef struct WoodyLexer
{
    char * input;
    uint32_t linenumber;
    uint32_t column_number;
    uint32_t position;

    WdyToken current;
    WdyToken lookahead;
} WdyLexer;


WdyLexer * wdy_lexer_new();
void wdy_lexer_free(WdyLexer * lexer);

void wdy_lexer_set_input(WdyLexer * lexer, char * input);

WdyTokenType wdy_lexer_next(WdyLexer * lexer);
WdyTokenType wdy_lexer_peek(WdyLexer * lexer);

#endif
