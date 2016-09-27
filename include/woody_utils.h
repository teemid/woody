#ifndef WOODY_UTILS_H
#define WOODY_UTILS_H

#include "woody_common.h"


#define DECLARE_BUFFER(name, type)                                        \
    typedef struct name##Buffer                                           \
    {                                                                     \
        type * values;                                                    \
        size_t count;                                                     \
        size_t capacity;                                                  \
    } name##Buffer;                                                       \
                                                                          \
    name##Buffer * name##BufferNew (size_t initial_capacity);             \
    void name##BufferResize (name##Buffer * buffer, size_t new_capacity); \
    void name##BufferFree (name##Buffer * buffer);                        \
    void name##BufferPush (name##Buffer * buffer, type value);            \
    type name##BufferPop  (name##Buffer * buffer)


#define DEFINE_BUFFER(name, type)                                               \
    name##Buffer * name##BufferNew (size_t initial_capacity)                    \
    {                                                                           \
        name##Buffer * buffer = (name##Buffer *)Allocate(sizeof(name##Buffer)); \
        buffer->values = Buffer(type, initial_capacity);                        \
        buffer->count = 0;                                                      \
        buffer->capacity = initial_capacity;                                    \
                                                                                \
        return buffer;                                                          \
    }                                                                           \
                                                                                \
    void name##BufferResize (name##Buffer * buffer, size_t new_capacity)        \
    {                                                                           \
        type * temp = ResizeBuffer(type, buffer->values, new_capacity);         \
        if (temp)                                                               \
        {                                                                       \
            buffer->values = temp;                                              \
            buffer->capacity = new_capacity;                                    \
        }                                                                       \
    }                                                                           \
                                                                                \
    void name##BufferFree (name##Buffer * buffer)                               \
    {                                                                           \
        Deallocate(buffer->values);                                             \
        Deallocate(buffer);                                                     \
    }                                                                           \
                                                                                \
    void name##BufferPush (name##Buffer * buffer, type value)                   \
    {                                                                           \
        buffer->values[buffer->count++] = value;                                \
                                                                                \
        if (buffer->count == buffer->capacity)                                  \
        {                                                                       \
            name##BufferResize(buffer, buffer->capacity * 2);                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    type name##BufferPop (name##Buffer * buffer)                                \
    {                                                                           \
        return buffer->values[buffer->count--];                                 \
    }


#define DECLARE_TABLE(name, key_type, value_type)                                             \
    typedef struct                                                                            \
    {                                                                                         \
        key_type key;                                                                         \
        uint32_t hash;                                                                        \
        value_type value;                                                                     \
    } name##Node;                                                                             \
                                                                                              \
    typedef struct                                                                            \
    {                                                                                         \
        name##Node * nodes;                                                                   \
        size_t count;                                                                         \
        size_t capacity;                                                                      \
    } name##Table;                                                                            \
                                                                                              \
    name##Table * name##TableNew (size_t initial_capacity);                                   \
    void name##TableFree (name##Table * table);                                               \
    void name##TableAdd (name##Table * table, key_type key, uint32_t hash, value_type value); \
    name##Node * name##TableFind(name##Table * table, uint32_t hash);                         \
    void name##TableRemove (name##Table * table, uint32_t hash)


uint32_t djb2(const char * key, size_t length);


#define INVALID_HASH 0


#define DEFINE_TABLE(name, key_type, value_type)                                             \
    name##Table * name##TableNew (size_t initial_capacity)                                   \
    {                                                                                        \
        name##Table * table = (name##Table *)Allocate(sizeof(name##Table));                  \
        table->nodes = Buffer(name##Node, initial_capacity);                                 \
        table->count = 0;                                                                    \
        table->capacity = initial_capacity;                                                  \
                                                                                             \
        for (uint32_t i = 0; i < table->capacity; i++)                                       \
        {                                                                                    \
            name##Node * node = table->nodes + i;                                            \
            node->hash = INVALID_HASH;                                                       \
        }                                                                                    \
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
        uint32_t i = 1;                                                                      \
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
        uint32_t i = 1;                                                                      \
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


char * ReadFile (const char * filename);

typedef struct WoodyState WoodyState;

#endif
