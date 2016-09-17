#include <stdio.h>

#include "woody_utils.h"
#include "woody_opcodes.h"
#include "woody_memory.h"


uint32_t djb2 (char * key, size_t length)
{
    uint32_t hash = 5381;

    for (size_t i = 0; i < length; i++)
    {
        hash = ((hash << 5) + hash) + key[i];
    }

    return hash;
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

    char * buffer = (char *)ZeroAllocate(sizeof(char), size + 1);
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
