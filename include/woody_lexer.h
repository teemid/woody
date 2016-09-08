#ifndef WOODY_LEXER_H
#define WOODY_LEXER_H

#include "stdlib.h"
#include "stdint.h"

typedef enum
{
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERIX,
    TOKEN_SLASH,
    TOKEN_VAR,
    TOKEN_EQ,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_EOF
} WoodyTokenType;


extern const char * woody_tokens[];


typedef struct
{
    WoodyTokenType type;
    double value;
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

void WoodyLexerSetInput (WoodyLexer * lexer, char * input);

WoodyTokenType WoodyLexerNext(WoodyLexer * lexer);
WoodyTokenType WoodyLexerPeek (WoodyLexer * lexer);

#endif
