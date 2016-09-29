#include "string.h"
#include "stdio.h"

#include "woody_lexer.h"
#include "woody_memory.h"


const char * woody_tokens[] = {
    #define STR(s) #s
    #define TOKEN(t) STR(TOKEN_##t),
    #include "woody_tokens.def"
    #undef STR
};


typedef struct
{
    const char * keyword;
    size_t length;
} Keyword;


Keyword keywords[] = {
    #define KEYWORD(kw, str, length) { #str, length },
    #include "woody_tokens.def"
};


static uint32_t keyword_count = sizeof(keywords) / sizeof(Keyword);


#define SetToken(lexer, start_pos, token_type) \
    lexer->current.type = (token_type); \
    lexer->current.start = (start_pos); \
    lexer->current.length = 1


WoodyLexer * WoodyLexerNew ()
{
    WoodyLexer * lexer = (WoodyLexer *)Allocate(sizeof(WoodyLexer));
    lexer->position = 0;
    lexer->linenumber = 0;
    lexer->column_number = 0;
    lexer->input = NULL;
    lexer->current.type = TOKEN_EOF;
    lexer->lookahead.type = TOKEN_EOF;

    return lexer;
}


void WoodyLexerFree (WoodyLexer * lexer)
{
    Deallocate(lexer);
}


void WoodyLexerSetInput (WoodyLexer * lexer, char * input)
{
    lexer->input = input;
}


static void Lex (WoodyLexer * lexer)
{
    char c;

    for (;;)
    {
        switch (c = *(lexer->input + lexer->position))
        {
            case ' ': case '\t':
            {
                lexer->column_number++;
                lexer->position++;
            } break;
            case '\r':
            {
                lexer->position++;
            }
            case '\n':
            {
                lexer->linenumber++;
                lexer->position++;
                SetToken(lexer, lexer->input + lexer->position, TOKEN_NEWLINE);
            } return;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
            case 'Y': case 'Z':
            case '_':
            {
                char * start = lexer->input + lexer->position;

                while ('a' <= c && c <= 'z')
                {
                    lexer->position++;
                    lexer->column_number++;
                    c = *(lexer->input + lexer->position);
                }

                char * stop = lexer->input + lexer->position;

                ptrdiff_t length = stop - start;

                for (uint32_t i = 0; i < keyword_count; i++)
                {
                    if ((uint32_t)length == keywords[i].length && strncmp(start, keywords[i].keyword, length) == 0)
                    {
                        SetToken(lexer, start, TOKEN_VAR + i);
                        lexer->current.length = length;
                        return;
                    }
                }

                SetToken(lexer, start, TOKEN_IDENTIFIER);
                lexer->current.length = length;
            } return;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            {
                char * start = lexer->input + lexer->position;

                while (('0' <= c && c <= '9') || c == '.')
                {
                    lexer->position++;
                    lexer->column_number++;
                    c = *(lexer->input + lexer->position);
                }

                char * stop = lexer->input + lexer->position;

                char * end;

                lexer->current.value.number = strtod(start, &end);

                if (stop != end)
                {
                    printf("lexer and strtod not agreeing.\n");
                }

                ptrdiff_t diff = stop - start;

                SetToken(lexer, start, TOKEN_NUMBER);
                lexer->current.length = diff;
            } return;
            case ',':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_COMMA);
                lexer->position++;
            } return;
            case '+':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_PLUS);
                lexer->position++;
            } return;
            case '-':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_MINUS);
                lexer->position++;
            } return;
            case '*':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_ASTERIX);
                lexer->position++;
            } return;
            case '/':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_SLASH);
                lexer->position++;
            } return;
            case '=':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_EQ);
                lexer->position++;
            } return;
            case '(':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_OPEN_PAREN);
                lexer->position++;
            } return;
            case ')':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_CLOSE_PAREN);
                lexer->position++;
            } return;
            case '\0':
            {
                SetToken(lexer, lexer->input + lexer->position, TOKEN_EOF);
            } return;
        }
    }
}


WoodyTokenType WoodyLexerNext(WoodyLexer * lexer)
{
    if (lexer->lookahead.type == TOKEN_EOF)
    {
        Lex(lexer);
    }
    else
    {
        lexer->current = lexer->lookahead;
        lexer->lookahead.type = TOKEN_EOF;
    }

    return lexer->current.type;
}


WoodyTokenType WoodyLexerPeek (WoodyLexer * lexer)
{
    if (lexer->lookahead.type == TOKEN_EOF)
    {
        WoodyToken token = lexer->current;

        WoodyLexerNext(lexer);

        lexer->lookahead = lexer->current;
        lexer->current = token;
    }

    return lexer->lookahead.type;
}
