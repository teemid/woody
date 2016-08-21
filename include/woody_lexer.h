#ifndef WOODY_LEXER_H
#define WOODY_LEXER_H

typedef enum
{
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERIX,
    TOKEN_FORWARD_SLASH,
    TOKEN_NUMBER,
    TOKEN_EQUALITY
} WoodyTokenType;


extern const char * woody_tokens[];


typedef struct
{
    char * name;
    size_t length;
    uint32_t hash;
} Keyword;


typedef struct
{
    WoodyTokenType type;
    double value;
    const char * start;
    uint32_t linenumber;
    size_t length;
} WoodyToken;


typedef struct
{
    uint32_t position;
    uint32_t column_number;
    uint32_t linenumber;

    WoodyToken token;
    WoodyToken lookahead;

    char * input;
} WoodyLexer;


void WoodyLex(WoodyLexer * lexer);
void WoodyLex(WoodyLexer * lexer, char * input);

WoodyTokenType WoodyLexerNext(WoodyLexer * lexer);
WoodyTokenType WoodyLexerPeek(WoodyLexer * lexer);



#endif
