#ifndef WOODY_UTILS_H
#define WOODY_UTILS_H

#include "woody_common.h"


#define DECLARE_BUFFER(typename, prefix, type)                                   \
    typedef struct typename##Buffer                                              \
    {                                                                            \
        type * values;                                                           \
        size_t count;                                                            \
        size_t capacity;                                                         \
    } typename##Buffer;                                                          \
                                                                                 \
    typename##Buffer * prefix##_buffer_new(size_t initial_capacity);             \
    void prefix##_buffer_resize(typename##Buffer * buffer, size_t new_capacity); \
    void prefix##_buffer_free(typename##Buffer * buffer);                        \
    void prefix##_buffer_push(typename##Buffer * buffer, type value);            \
    type prefix##_buffer_pop(typename##Buffer * buffer)


#define DEFINE_BUFFER(typename, prefix, type)                                                   \
    typename##Buffer * prefix##_buffer_new(size_t initial_capacity)                             \
    {                                                                                           \
        typename##Buffer * buffer = (typename##Buffer *)wdy_allocate(sizeof(typename##Buffer)); \
        buffer->values = wdy_allocate_buffer(type, initial_capacity);                           \
        buffer->count = 0;                                                                      \
        buffer->capacity = initial_capacity;                                                    \
                                                                                                \
        return buffer;                                                                          \
    }                                                                                           \
                                                                                                \
    void prefix##_buffer_resize(typename##Buffer * buffer, size_t new_capacity)                 \
    {                                                                                           \
        type * temp = wdy_resize_buffer(type, buffer->values, new_capacity);                    \
        if (temp)                                                                               \
        {                                                                                       \
            buffer->values = temp;                                                              \
            buffer->capacity = new_capacity;                                                    \
        }                                                                                       \
    }                                                                                           \
                                                                                                \
    void prefix##_buffer_free(typename##Buffer * buffer)                                        \
    {                                                                                           \
        wdy_deallocate(buffer->values);                                                         \
        wdy_deallocate(buffer);                                                                 \
    }                                                                                           \
                                                                                                \
    void prefix##_buffer_push(typename##Buffer * buffer, type value)                            \
    {                                                                                           \
        buffer->values[buffer->count++] = value;                                                \
                                                                                                \
        if (buffer->count == buffer->capacity)                                                  \
        {                                                                                       \
            prefix##_buffer_resize(buffer, buffer->capacity * 2);                               \
        }                                                                                       \
    }                                                                                           \
                                                                                                \
    type prefix##_buffer_pop(typename##Buffer * buffer)                                         \
    {                                                                                           \
        return buffer->values[buffer->count--];                                                 \
    }


#define DECLARE_TABLE(typename, prefix, key_type, value_type)                                        \
    typedef struct                                                                                   \
    {                                                                                                \
        key_type key;                                                                                \
        uint32_t hash;                                                                               \
        value_type value;                                                                            \
    } typename##Node;                                                                                \
                                                                                                     \
    typedef struct                                                                                   \
    {                                                                                                \
        typename##Node * nodes;                                                                      \
        size_t count;                                                                                \
        size_t capacity;                                                                             \
    } typename##Table;                                                                               \
                                                                                                     \
    typename##Table * prefix##_table_new (size_t initial_capacity);                                  \
    void prefix##_table_resize(typename##Table * table, size_t new_capacity);                        \
    void prefix##_table_free(typename##Table * table);                                               \
    void prefix##_table_add(typename##Table * table, key_type key, uint32_t hash, value_type value); \
    typename##Node * prefix##_table_find(typename##Table * table, uint32_t hash);                    \
    void prefix##_table_remove(typename##Table * table, uint32_t hash)


uint32_t djb2(const char * key, size_t length);


#define INVALID_HASH 0


#define DEFINE_TABLE(typename, prefix, key_type, value_type)                                        \
    typename##Table * prefix##_table_new(size_t initial_capacity)                                   \
    {                                                                                               \
        typename##Table * table = (typename##Table *)wdy_zero_allocate(sizeof(typename##Table), 1); \
        prefix##_table_resize(table, initial_capacity);                                             \
                                                                                                    \
        for (uint32_t i = 0; i < table->capacity; i++)                                              \
        {                                                                                           \
            typename##Node * node = table->nodes + i;                                               \
            node->hash = INVALID_HASH;                                                              \
        }                                                                                           \
                                                                                                    \
        return table;                                                                               \
    }                                                                                               \
                                                                                                    \
    void prefix##_table_free(typename##Table * table)                                               \
    {                                                                                               \
        wdy_deallocate(table->nodes);                                                               \
        wdy_deallocate(table);                                                                      \
    }                                                                                               \
                                                                                                    \
    void prefix##_table_resize(typename##Table * table, size_t capacity)                            \
    {                                                                                               \
        typename##Node * nodes = table->nodes;                                                      \
        size_t old_capacity = table->capacity;                                                      \
                                                                                                    \
        table->nodes = wdy_allocate_buffer(typename##Node, capacity);                               \
        wdy_zero(table->nodes, sizeof(typename##Node) * capacity);                                  \
        table->capacity = capacity;                                                                 \
                                                                                                    \
        typename##Node * node = NULL;                                                               \
        for (size_t i = 0; i < old_capacity; i++)                                                   \
        {                                                                                           \
            node = nodes + i;                                                                       \
                                                                                                    \
            if (node->hash != INVALID_HASH)                                                         \
            {                                                                                       \
                uint32_t index = node->hash % capacity;                                             \
                typename##Node * new_node = table->nodes + index;                                   \
                new_node->hash = node->hash;                                                        \
                new_node->key = node->key;                                                          \
                new_node->value = node->value;                                                      \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (nodes)                                                                                  \
        {                                                                                           \
            wdy_deallocate(nodes);                                                                  \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    static void prefix##_table_check_size(typename##Table * table)                                  \
    {                                                                                               \
        float fillRate = table->count / (float)table->capacity;                                     \
                                                                                                    \
        if (fillRate < 0.7)                                                                         \
        {                                                                                           \
            return;                                                                                 \
        }                                                                                           \
                                                                                                    \
        prefix##_table_resize(table, table->capacity * 2);                                          \
    }                                                                                               \
                                                                                                    \
    typename##Node * prefix##_table_find(typename##Table * table, uint32_t hash)                    \
    {                                                                                               \
        uint32_t index = hash % table->capacity;                                                    \
        typename##Node * node = table->nodes + index;                                               \
                                                                                                    \
        uint32_t i = 1;                                                                             \
        while (node->hash != INVALID_HASH && node->hash != hash)                                    \
        {                                                                                           \
            node = table->nodes + (index + i * i);                                                  \
        }                                                                                           \
                                                                                                    \
        return node;                                                                                \
    }                                                                                               \
                                                                                                    \
    void prefix##_table_add(typename##Table * table, key_type key, uint32_t hash, value_type value) \
    {                                                                                               \
        uint32_t index = hash % table->capacity;                                                    \
        typename##Node * node = table->nodes + index;                                               \
                                                                                                    \
        uint32_t i = 1;                                                                             \
        while (node->hash != INVALID_HASH && node->hash != hash)                                    \
        {                                                                                           \
            index = (hash + i * i) % table->capacity;                                               \
            node = table->nodes + index;                                                            \
                                                                                                    \
            i += 1;                                                                                 \
        }                                                                                           \
                                                                                                    \
        table->count = (node->hash == hash) ? table->count : table->count + 1;                      \
                                                                                                    \
        node->hash = hash;                                                                          \
        node->key = key;                                                                            \
        node->value = value;                                                                        \
                                                                                                    \
        prefix##_table_check_size(table);                                                           \
    }                                                                                               \
                                                                                                    \
    void prefix##_table_remove(typename##Table * table, uint32_t hash)                              \
    {                                                                                               \
        typename##Node * node = prefix##_table_find(table, hash);                                   \
                                                                                                    \
        if (node)                                                                                   \
        {                                                                                           \
            node->hash = INVALID_HASH;                                                              \
        }                                                                                           \
    }


char * read_file(const char * filename);

typedef struct WoodyState WdyState;

#endif
