#include <stdio.h>

#if _WIN32
#include <io.h>
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define access(...) _access(__VA_ARGS__)
#endif

#include "woody_common.h"
#include "woody_memory.h"
#include "woody_opcodes.h"
#include "woody_state.h"
#include "woody_utils.h"


uint32_t djb2 (const char * key, size_t length)
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
    if (access(filename, F_OK))
    {
        Log("File %s is not accessible.\n", filename);

        exit(1);
    }

    FILE * file = fopen(filename, "rb");

    if (!file)
    {
        Log("Failed to read file: %s.\n", filename);

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
        Log("Read error.\n");

        Deallocate(buffer);

        return NULL;
    }

    fclose(file);

    return buffer;
}
