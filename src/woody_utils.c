#include <stdio.h>

#include "woody_state.h"
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
