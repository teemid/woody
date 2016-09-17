#include "stdio.h"

#include "woody_lexer.h"
#include "woody_memory.h"
#include "woody_parser.h"
#include "woody_state.h"
#include "woody_utils.h"
#include "woody_vm.h"


int main (int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("Usage: \nwoody <filename>");

        return 1;
    }

    char * source = NULL;
    source = ReadFile(argv[1]);

    printf("%s\n\n", source);

    WoodyLexer * lexer = WoodyLexerNew();
    WoodyState * state = WoodyNewState();

    WoodyLexerSetInput(lexer, source);

    // while (WoodyLexerNext(lexer) != TOKEN_EOF)
    // {
    //     printf("%s\n", woody_tokens[lexer->current.type]);
    // }

    WoodyParse(state, lexer);

    PrintInstructions(state->code);

    WoodyRun(state);

    Deallocate(source);
    Deallocate(lexer);

    return 0;
}
