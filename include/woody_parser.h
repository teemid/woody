#ifndef WOODY_PARSER_H
#define WOODY_PARSER_H


typedef struct WoodyState WoodyState;
typedef struct WoodyLexer WoodyLexer;


void WoodyParse (WoodyState * state, WoodyLexer * lexer);


#endif
