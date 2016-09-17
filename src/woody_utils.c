#include <stdio.h>

#include "woody_utils.h"
#include "woody_opcodes.h"
#include "woody_memory.h"


#define DEFINE_BUFFER(name, type)                                                                  \
    name##Buffer * name##BufferNew (size_t initial_capacity)                                       \
    {                                                                                              \
        name##Buffer * buffer = (name##Buffer *)Allocate(sizeof(name##Buffer));                    \
        buffer->values = Buffer(type, initial_capacity);                                           \
        buffer->count = 0;                                                                         \
        buffer->capacity = initial_capacity;                                                       \
                                                                                                   \
        return buffer;                                                                             \
    }                                                                                              \
                                                                                                   \
    void name##BufferFree (name##Buffer * buffer)                                                  \
    {                                                                                              \
        Deallocate(buffer->values);                                                                \
        Deallocate(buffer);                                                                        \
    }                                                                                              \
                                                                                                   \
    void name##BufferPush (name##Buffer * buffer, type value)                                      \
    {                                                                                              \
        buffer->values[buffer->count++] = value;                                                   \
                                                                                                   \
        if (buffer->count == buffer->capacity)                                                     \
        {                                                                                          \
            type * temp = ReallocateBuffer(type, buffer->values, buffer->capacity * 2);            \
            if (temp)                                                                              \
            {                                                                                      \
                buffer->values = temp;                                                             \
                buffer->capacity = buffer->capacity * 2;                                           \
            }                                                                                      \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    type name##BufferPop (name##Buffer * buffer)                                                   \
    {                                                                                              \
        return buffer->values[buffer->count--];                                                    \
    }


#define INVALID_HASH 0


static uint32_t djb2 (char * key, size_t length)
{
    uint32_t hash = 5381;

    for (size_t i = 0; i < length; i++)
    {
        hash = ((hash << 5) + hash) + key[i];
    }

    return hash;
}


#define Hash(key, length) djb2(key, length)


#define DEFINE_TABLE(name, key_type, value_type)                                             \
    name##Table * name##TableNew (size_t initial_capacity)                                   \
    {                                                                                        \
        name##Table * table = (name##Table *)Allocate(sizeof(name##Table));                  \
        table->nodes = Buffer(name##Node, initial_capacity);                                 \
        table->count = 0;                                                                    \
        table->capacity = initial_capacity;                                                  \
                                                                                             \
        return table;                                                                        \
    }                                                                                        \
                                                                                             \
    void name##TableFree (name##Table * table)                                               \
    {                                                                                        \
        Deallocate(table->nodes);                                                            \
        Deallocate(table);                                                                   \
    }                                                                                        \
                                                                                             \
    void name##TableResize (name##Table * table, size_t capacity)                            \
    {                                                                                        \
        name##Node * nodes = table->nodes;                                                   \
        size_t old_capacity = table->capacity;                                               \
                                                                                             \
        table->nodes = Buffer(name##Node, capacity);                                         \
        table->capacity = capacity;                                                          \
                                                                                             \
        name##Node * node = NULL;                                                            \
        for (size_t i = 0; i < old_capacity; i++)                                            \
        {                                                                                    \
            node = nodes + i;                                                                \
                                                                                             \
            if (node->hash != INVALID_HASH)                                                  \
            {                                                                                \
                uint32_t index = node->hash % capacity;                                      \
                name##Node * new_node = table->nodes + index;                                \
                new_node->hash = node->hash;                                                 \
                new_node->key = node->key;                                                   \
                new_node->value = node->value;                                               \
            }                                                                                \
        }                                                                                    \
                                                                                             \
        Deallocate(nodes);                                                                   \
    }                                                                                        \
                                                                                             \
    static void name##TableCheckSize (name##Table * table)                                   \
    {                                                                                        \
        if (table->count < table->capacity)                                                  \
        {                                                                                    \
            return;                                                                          \
        }                                                                                    \
                                                                                             \
        name##TableResize(table, table->capacity * 2);                                       \
    }                                                                                        \
                                                                                             \
    name##Node * name##TableFind (name##Table * table, uint32_t hash)                        \
    {                                                                                        \
        uint32_t index = hash % table->capacity;                                             \
        name##Node * node = table->nodes + index;                                            \
                                                                                             \
        uint32_t i = 0;                                                                      \
        while (node->hash != INVALID_HASH && node->hash != hash)                             \
        {                                                                                    \
            node = table->nodes + (index + i * i);                                           \
        }                                                                                    \
                                                                                             \
        return node;                                                                         \
    }                                                                                        \
                                                                                             \
    void name##TableAdd (name##Table * table, key_type key, uint32_t hash, value_type value) \
    {                                                                                        \
        uint32_t index = hash % table->capacity;                                             \
        name##Node * node = table->nodes + index;                                            \
                                                                                             \
        uint32_t i = 0;                                                                      \
        while (node->hash != INVALID_HASH && node->hash != hash)                             \
        {                                                                                    \
            index = (hash + i * i) % table->capacity;                                        \
            node = table->nodes + index;                                                     \
                                                                                             \
            i += 1;                                                                          \
        }                                                                                    \
                                                                                             \
        table->count = (node->hash == hash) ? table->count : table->count + 1;               \
                                                                                             \
        node->hash = hash;                                                                   \
        node->key = key;                                                                     \
        node->value = value;                                                                 \
                                                                                             \
        name##TableCheckSize(table);                                                         \
    }                                                                                        \
                                                                                             \
    void name##TableRemove (name##Table * table, uint32_t hash)                              \
    {                                                                                        \
        name##Node * node = name##TableFind(table, hash);                                    \
                                                                                             \
        if (node)                                                                            \
        {                                                                                    \
            node->hash = INVALID_HASH;                                                       \
        }                                                                                    \
    }


DEFINE_BUFFER(Instruction, Instruction);
DEFINE_BUFFER(Value, double);

DEFINE_TABLE(Symbol, char *, uint32_t);

#define PrintBuffer(buffer, size)                                       \
    for (uint32_t i = 0; i < (size); i++)                               \
    {                                                                   \
        char c = buffer[i];                                             \
                                                                        \
        switch(c)                                                       \
        {                                                               \
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': \
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': \
            case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': \
            case 's': case 't': case 'u': case 'v': case 'w': case 'x': \
            case 'y': case 'z':                                         \
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': \
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': \
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': \
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': \
            case 'Y': case 'Z':                                         \
            case '_':                                                   \
            case '=': case '+': case '-': case '*': case '/': case '(': \
            case ')':                                                   \
            {                                                           \
                printf("%c\n", c);                                      \
            } break;                                                    \
            case '0': case '1': case '2': case '3': case '4': case '5': \
            case '6': case '7': case '8': case '9':                     \
            {                                                           \
                printf("Number: %i\n", atoi(&c));                       \
            } break;                                                    \
            case ' ':                                                   \
            {                                                           \
                printf("<Space>\n");                                    \
            } break;                                                    \
            default:                                                    \
            {                                                           \
                printf("%i\n", c);                                      \
            } break;                                                    \
        }                                                               \
    }


char * ReadFile (const char * filename)
{
    FILE * file = fopen(filename, "rb");

    if (!file)
    {
        printf("Failed to read file: %s.\n", filename);

        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char * buffer = calloc(sizeof(char), size + 1); // Buffer(char, size);
    size_t result = fread(buffer, 1, size, file);
    buffer[size] = 0;

    if (!result)
    {
        printf("Read error.\n");

        Deallocate(buffer);

        return NULL;
    }

    fclose(file);

    return buffer;
}


void PrintInstructions (InstructionBuffer * buffer)
{
    printf("InstructionBuffer: \n");

    for (uint32_t i = 0; i < buffer->count; i++)
    {
        uint32_t code = buffer->values[i];

        switch (code)
        {
            case OP_PLUS:
            case OP_MINUS:
            case OP_MULT:
            case OP_DIV:
            case OP_END:
            {
                printf("%s\n", woody_opcodes[code]);
            } break;

            case OP_CONSTANT:
            case OP_LOAD:
            case OP_STORE:
            {
                printf("%s %d\n", woody_opcodes[code], buffer->values[++i]);
            }
        }
    }

    printf("\n\n");
}
