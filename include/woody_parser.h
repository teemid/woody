#ifndef WOODY_PARSER_H
#define WOODY_PARSER_H


typedef struct WoodyState WdyState;
typedef struct WoodyLexer WdyLexer;


void wdy_parse(WdyState * state, WdyLexer * lexer);


#endif
