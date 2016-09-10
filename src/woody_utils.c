#include "stdlib.h"
#include "stdio.h"

#include "woody_utils.h"
#include "woody_opcodes.h"
#include "woody_memory.h"


#define DefineBuffer(name, type)                                                                   \
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


#define DefineTable(name, key_type, value_type)                                              \
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


DefineBuffer(Instruction, Instruction);
DefineBuffer(Value, double);

DefineTable(Symbol, char *, uint32_t);


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

    char * buffer = Buffer(char, size);
    buffer[size - 1] = '\0';

    size_t result = fread(buffer, 1, size, file);

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
