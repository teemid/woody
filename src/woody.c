#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

#include "woody.h"

/** DEFINE STATE **/
typedef uint32_t Instruction;


#define DeclareBuffer(type, name)                                                               \
    typedef struct                                                                              \
    {                                                                                           \
        type * values;                                                                          \
        size_t count;                                                                           \
        size_t capacity;                                                                        \
    } name##Buffer;                                                                             \
                                                                                                \
    name##Buffer * Initialize##name##Buffer (name##Buffer * buffer, size_t initial_capacity)    \
    {                                                                                           \
        buffer->values = malloc(sizeof(type) * initial_capacity);                               \
        buffer->count = 0;                                                                      \
        buffer->capacity = initial_capacity;                                                    \
                                                                                                \
        if (!buffer || !buffer->values)                                                         \
        {                                                                                       \
            printf("Failed to initialize buffer.\n");                                           \
            exit(1);                                                                            \
        }                                                                                       \
                                                                                                \
        return buffer;                                                                          \
    }                                                                                           \
                                                                                                \
    void Free##name##Buffer (name##Buffer * buffer)                                             \
    {                                                                                           \
        buffer->count = 0;                                                                      \
        buffer->capacity = 0;                                                                   \
        free(buffer->values);                                                                   \
    }                                                                                           \
                                                                                                \
    void name##BufferPush (name##Buffer * buffer, type value)                                   \
    {                                                                                           \
        buffer->values[buffer->count++] = value;                                                \
    }


DeclareBuffer(Instruction, Instruction);

DeclareBuffer(double, Value);

typedef struct
{
    Instruction * ip;
    InstructionBuffer code;
    ValueBuffer constants;
} WoodyState;

/** DEFINE STATE END **/


/** DEFINE UTILS **/
static char * ReadFile (const char * filename)
{
    FILE * file = fopen(filename, "r");

    if (!file)
    {
        printf("Failed to read file: %s.\n", filename);

        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char * buffer = malloc(sizeof(char) * size);
    buffer[size - 1] = '\0';

    size_t result = fread(buffer, 1, size, file);

    if (!result)
    {
        printf("Read error.\n");

        free(buffer);

        return NULL;
    }

    fclose(file);

    return buffer;
}
/** DEFINE UTILS END **/

/** DEFINE TOKENIZER **/
typedef enum
{
    TOKEN_VAR,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERIX,
    TOKEN_SLASH,
    TOKEN_EQ,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_EOF
} WoodyTokenType;


const char * woody_tokens[] = {
    "TOKEN_VAR",
    "TOKEN_PLUS",
    "TOKEN_MINUS",
    "TOKEN_ASTERIX",
    "TOKEN_SLASH",
    "TOKEN_EQ",
    "TOKEN_OPEN_PAREN",
    "TOKEN_CLOSE_PAREN",
    "TOKEN_NUMBER",
    "TOKEN_IDENTIFIER",
    "TOKEN_EOF"
};


typedef struct
{
    const char * keyword;
    size_t length;
} Keyword;

static uint32_t keyword_count = 1;

Keyword keywords[] = {
    { "var", 3 }
};


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


#define SetToken(lexer, start_pos, token_type) \
    lexer->current.type = (token_type); \
    lexer->current.start = (start_pos); \
    lexer->current.length = 1


static void WoodyLexerNext (WoodyLexer * lexer)
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
            case '\n':
            {
                lexer->linenumber++;
                lexer->position++;
            } break;
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
                    if ((uint32_t)length == keywords[i].length && strstr(start, keywords[i].keyword) != 0)
                    {
                        SetToken(lexer, start, TOKEN_VAR);
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

                while ('0' <= c && c <= '9' || c == '.')
                {
                    lexer->position++;
                    lexer->column_number++;
                    c = *(lexer->input + lexer->position);
                }

                char * stop = lexer->input + lexer->position;

                char * end;

                lexer->current.value = strtod(start, &end);

                if (stop != end)
                {
                    printf("lexer and strtod not agreeing.\n");
                }

                ptrdiff_t diff = stop - start;

                SetToken(lexer, start, TOKEN_NUMBER);
                lexer->current.length = diff;
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
/** DEFINE TOKENIZER END **/

/** DEFINE OPCODES **/
typedef enum
{
    OP_LOAD,
    OP_STORE,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
} Opcode;
/** DEFINE OPCODES END **/


/** DEFINE PARSER **/
typedef void (* GrammarFunction) (WoodyState * state);


typedef struct
{
    GrammarFunction prefix;
    GrammarFunction postfix;
} WoodyRules;


typedef enum
{
    PRECEDENCE_NONE,
    PRECEDENCE_LOWEST,      // Sentinel
    PRECEDENCE_ASSIGNMENT,  // =
    PRECEDENCE_LOGICAL_OR,  // ||
    PRECEDENCE_LOGICAL_AND, // &&
    PRECEDENCE_EQUALITY,    // == !=
    PRECEDENCE_COMPARISON,  // < > <= >=
    PRECEDENCE_TERM,        // + -
    PRECEDENCE_FACTOR,      // * /
    PRECEDENCE_UNARY,       // - !
    PRECEDENCE_CALL,        // . () [] {}
} Precedence;


typedef void (* GrammarFn) (WoodyState * state, WoodyLexer * lexer);

typedef struct
{
    GrammarFn prefix;
    GrammarFn infix;
    Precedence precedence;
    char * name;
} GrammarRule;


static void UnaryOperator (WoodyState * state, WoodyLexer * lexer);
static void InfixOperator (WoodyState * state, WoodyLexer * lexer);
static void VarStatement  (WoodyState * state, WoodyLexer * lexer);
static void Identifier    (WoodyState * state, WoodyLexer * lexer);
static void Literal       (WoodyState * state, WoodyLexer * lexer);

#undef UNUSED
#define UNUSED                           { NULL,          NULL,          PRECEDENCE_NONE, NULL }
#define PREFIX(fn)                       { fn,            NULL,          PRECEDENCE_NONE, NULL }
#define INFIX(precedence, fn)            { NULL,          fn,            precedence,      NULL }
#define PREFIX_OPERATOR(name)            { UnaryOperator, NULL,          PRECEDENCE_NONE, name }
#define INFIX_OPERATOR(name, precedence) { NULL,          InfixOperator, precedence,      name }
#define OPERATOR(name)                   { UnaryOperator, InfixOperator, PRECEDENCE_TERM, name }


// NOTE (Emil): Each rule is associated with a token.
GrammarRule rules[] = {
    /* TOKEN_VAR         */ PREFIX(VarStatement),
    /* TOKEN_PLUS        */ INFIX_OPERATOR("+", PRECEDENCE_TERM),
    /* TOKEN_MINUS       */ OPERATOR("-"),
    /* TOKEN_ASTERIX     */ INFIX_OPERATOR("*", PRECEDENCE_FACTOR),
    /* TOKEN_SLASH       */ INFIX_OPERATOR("/", PRECEDENCE_FACTOR),
    /* TOKEN_EQ          */ INFIX_OPERATOR("=", PRECEDENCE_ASSIGNMENT),
    /* TOKEN_OPEN_PAREN  */ UNUSED,
    /* TOKEN_CLOSE_PAREN */ UNUSED,
    /* TOKEN_NUMBER      */ PREFIX(Literal),
    /* TOKEN_IDENTIFIER  */ PREFIX(Identifier),
    /* TOKEN_EOF         */ UNUSED,
};

#undef UNUSED
#define UNUSED(ptr) (void)(ptr)

#define PrintToken(lexer) printf( \
    "%.*s %s\n", \
    (lexer)->current.length, \
    (lexer)->current.start, \
    woody_tokens[(lexer)->current.type])


WoodyState * WoodyNewState ()
{
    WoodyState * state = malloc(sizeof(WoodyState));
    InitializeInstructionBuffer(&state->code, 40);
    InitializeValueBuffer(&state->constants, 40);

    return state;
}


#define UNUSED(ptr) (void)(ptr)


static void Statement (WoodyState * state, WoodyLexer * lexer)
{
    GrammarFn prefix = rules[lexer->current.type].prefix;

    if (!prefix)
    {
        printf("Expected prefix!\n");
        exit(1);
    }

    prefix(state, lexer);
}


static uint32_t AddLocalVariable (WoodyState * state, WoodyLexer * lexer)
{
    UNUSED(state);
    UNUSED(lexer);

    return 0;
}


static void Expression (WoodyState * state, WoodyLexer * lexer)
{
    GrammarFn prefix = rules[lexer->current.type].prefix;

    if (!prefix)
    {
        printf("Expected prefix in expression");
        exit(1);
    }

    prefix(state, lexer);
}


static void VarStatement (WoodyState * state, WoodyLexer * lexer)
{
    PrintToken(lexer);

    if (lexer->current.type != TOKEN_VAR)
    {
        printf("Expected var statement!\n");
        exit(1);
    }

    WoodyLexerNext(lexer);
    PrintToken(lexer);

    if (lexer->current.type != TOKEN_IDENTIFIER)
    {
        printf("Expected identifier!\n");
        exit(1);
    }

    /*
     * NOTE (Emil):
     * Here we need a temp structure to hold identifier -> variable number.
     */
    uint32_t local = AddLocalVariable(state, lexer);


    /*
     * NOTE (Emil):
     * Implement lookahead and check if the next token is '='.
     * If it is we go to assignment. If not return, the statement
     * declared a variable and is done.
     */

    #define WoodyLexerPeek(lexer) (lexer)
    WoodyLexerPeek(lexer);

    if (lexer->lookahead.type == TOKEN_EQ)
    {
        WoodyLexerNext(lexer);
        WoodyLexerNext(lexer);
        Expression(state, lexer);
    }


    InstructionBufferPush(&state->code, OP_STORE);
    InstructionBufferPush(&state->code, local);
}


static void UnaryOperator (WoodyState * state, WoodyLexer * lexer)
{
    UNUSED(state);
    UNUSED(lexer);
}


static void InfixOperator (WoodyState * state, WoodyLexer * lexer)
{
    UNUSED(state);
    UNUSED(lexer);
}


static void Identifier (WoodyState * state, WoodyLexer * lexer)
{
    UNUSED(state);
    UNUSED(lexer);
}


static void Literal (WoodyState * state, WoodyLexer * lexer)
{
    UNUSED(state);
    UNUSED(lexer);
}


static void WoodyParse (WoodyState * state, WoodyLexer * lexer)
{
    WoodyLexerNext(lexer);

    while (lexer->current.type != TOKEN_EOF)
    {
        Statement(state, lexer);

        WoodyLexerNext(lexer);
    }

    PrintToken(lexer);
}
/** DEFINE PARSER END **/

/** DEFINE COMPILER **/
void WoodyCompile (WoodyState * state)
{
    UNUSED(state);
}
/** DEFINE COMPILER END **/

/** DEFINE VM **/
void WoodyRun (WoodyState * state)
{
    UNUSED(state);
}
/** DEFINE VM END**/


int main (int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("Usage: \nwoody <filename>");
    }

    char * source = NULL;
    source = ReadFile(argv[1]);

    printf("%s\n\n", source);

    WoodyLexer * lexer = malloc(sizeof(WoodyLexer));
    lexer->position = 0;
    lexer->linenumber = 0;
    lexer->column_number = 0;
    lexer->input = source;

    lexer->current.type = TOKEN_EOF;

    WoodyState * state = NULL;
    state = WoodyNewState();

    WoodyParse(state, lexer);

    free(source);
    free(lexer);
}
