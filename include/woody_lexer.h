#ifndef WOODY_LEXER_H
#define WOODY_LEXER_H

#include "woody_common.h"

typedef enum
{
    #define TOKEN(token) TOKEN_##token,
    #include "woody_tokens.h"
    #undef TOKEN
} WoodyTokenType;


extern const char * woody_tokens[];


typedef union {
    double number;
    uint32_t boolean;
} Value;


typedef struct
{
    WoodyTokenType type;
    Value value;
    const char * start;
    size_t length;
} WoodyToken;


typedef struct
{
    char * input;
    uint32_t linenumber;
    uint32_t column_number;
    uint32_t position;

    WoodyToken current;
    WoodyToken lookahead;
} WoodyLexer;


WoodyLexer * WoodyLexerNew ();
void WoodyLexerFree (WoodyLexer * lexer);

void WoodyLexerSetInput (WoodyLexer * lexer, char * input);

WoodyTokenType WoodyLexerNext (WoodyLexer * lexer);
WoodyTokenType WoodyLexerPeek (WoodyLexer * lexer);

#endif
