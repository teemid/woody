#include "stdio.h"

#include "woody_utils.h"
#include "woody_lexer.h"
#include "woody_state.h"
#include "woody_parser.h"


/** DEFINE OPCODES **/

/** DEFINE OPCODES END **/


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

    WoodyParse(state, lexer);

    free(source);
    free(lexer);

    return 0;
}
