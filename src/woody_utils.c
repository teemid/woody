#include "stdlib.h"
#include "stdio.h"

#include "woody_utils.h"


#define DefineBuffer(name, type)                                            \
    name##Buffer * name##BufferNew (size_t initial_capacity)                \
    {                                                                       \
        name##Buffer * buffer = malloc(sizeof(name##Buffer));                 \
        buffer->values = malloc(sizeof(type) * initial_capacity);           \
        buffer->count = 0;                                                  \
        buffer->capacity = initial_capacity;                                \
        \
        return buffer; \
    }                                                                       \
                                                                            \
    void name##BufferFree (name##Buffer * buffer)                           \
    {                                                                       \
        free(buffer->values);                                               \
        free(buffer);                                                       \
    }                                                                       \
                                                                            \
    void name##BufferPush (name##Buffer * buffer, type value)               \
    {                                                                       \
        buffer->values[buffer->count++] = value;                            \
                                                                            \
        if (buffer->count == buffer->capacity)                              \
        {                                                                   \
            buffer->values = realloc(buffer->values, buffer->capacity * 2); \
            buffer->capacity = buffer->capacity * 2;                        \
        }                                                                   \
    }                                                                       \
                                                                            \
    type name##BufferPop (name##Buffer * buffer)                \
    {                                                                       \
        return buffer->values[buffer->count--];                             \
    }

#define INVALID_HASH 0

#define DefineTable(name, key_type, value_type) \
    name##Table * name##TableNew (size_t initial_capacity) \
    { \
        name##Table * table = malloc(sizeof(name##Table)); \
        table->nodes = malloc(sizeof(name##Node) * initial_capacity); \
        table->count = 0; \
        table->capacity = initial_capacity; \
        \
        return table; \
    } \
    \
    void name##TableFree (name##Table * table) \
    { \
        free(table->nodes); \
        free(table); \
    } \
    \
    void name##TableAdd (name##Table * table, key_type key, uint32_t hash, value_type value) \
    { \
        *table; key; hash; value;\
    } \
    \
    void name##TableRemove (name##Table * table, key_type key, uint32_t hash) \
    { \
        *table; key; hash; \
    }

DefineBuffer(Instruction, Instruction);
DefineBuffer(Value, double);

DefineTable(Symbol, char *, uint32_t);

/*
#define DeclareTable(name, key_type, value_type)                                             \
    typedef struct                                                                           \
    {                                                                                        \
        key_type key;                                                                        \
        uint32_t hash;                                                                       \
        value_type value;                                                                    \
    } name##Node;                                                                            \
                                                                                             \
    typedef struct                                                                           \
    {                                                                                        \
        name##Node * nodes;                                                                  \
        size_t count;                                                                        \
        size_t capacity;                                                                     \
    } name##Table;                                                                           \
                                                                                             \
    void Initialize##name##Table (name##Table * table, size_t initial_capacity)              \
    {                                                                                        \
        table->nodes = malloc(sizeof(name##Node) * initial_capacity);                        \
                                                                                             \
        for (size_t i = 0; i < initial_capacity; i++)                                        \
        {                                                                                    \
            table->nodes->hash = INVALID_HASH;                                               \
        }                                                                                    \
                                                                                             \
        table->count = 0;                                                                    \
        table->capacity = initial_capacity;                                                  \
    }                                                                                        \
                                                                                             \
    void name##TableAdd (name##Table * table, key_type key, uint32_t hash, value_type value) \
    {                                                                                        \
        size_t index = hash % table->capacity;                                               \
        name##Node * node = table->nodes + index;                                            \
                                                                                             \
        size_t offset = 1;                                                                   \
        while (node->hash != INVALID_HASH)                                                   \
        {                                                                                    \
            node = table->nodes + index + offset * offset;                                   \
        }                                                                                    \
                                                                                             \
        node->hash = hash;                                                                   \
        node->key = key;                                                                     \
        node->value = value;                                                                 \
    }
*/


char * ReadFile (const char * filename)
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
