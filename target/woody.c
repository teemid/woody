#include <stdio.h>

#include "woody_lexer.h"
#include "woody_memory.h"
#include "woody_parser.h"
#include "woody_state.h"
#include "woody_utils.h"
#include "woody_vm.h"


void wdy_run_file(char * filename)
{
    char * source = read_file(filename);

    WdyLexer * lexer = wdy_lexer_new();
    WdyState * state = wdy_state_new();

    wdy_lexer_set_input(lexer, source);

    wdy_parse(state, lexer);

    wdy_run(state);

    wdy_deallocate(source);
    wdy_deallocate(lexer);
}


int main (int argc, char ** argv)
{
    if (argc < 2)
    {
        LOG("Usage: \nwoody <filename>");

        return 1;
    }

    wdy_run_file(argv[1]);

    return 0;
}
