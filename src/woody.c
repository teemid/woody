#include "stdio.h"

#include "woody_utils.h"
#include "woody_lexer.h"
#include "woody_state.h"
#include "woody_parser.h"
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

    WoodyParse(state, lexer);

    PrintInstructions(state->code);

    WoodyRun(state);

    free(source);
    free(lexer);

    return 0;
}
