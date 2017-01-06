#include <stdio.h>

#include "woody_lexer.h"
#include "woody_memory.h"
#include "woody_parser.h"
#include "woody_state.h"
#include "woody_utils.h"
#include "woody_vm.h"


void WoodyRunFile(char * filename)
{
    char * source = ReadFile(filename);

    WoodyLexer * lexer = WoodyLexerNew();
    WoodyState * state = WoodyNewState();

    WoodyLexerSetInput(lexer, source);

    WoodyParse(state, lexer);

    WoodyRun(state);

    Deallocate(source);
    Deallocate(lexer);
}


int main (int argc, char ** argv)
{
    if (argc < 2)
    {
        Log("Usage: \nwoody <filename>");

        return 1;
    }

    WoodyRunFile(argv[1]);

    return 0;
}
