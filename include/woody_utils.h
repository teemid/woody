#ifndef WOODY_UTILS_H
#define WOODY_UTILS_H

#include "stdint.h"

typedef uint32_t Instruction;


#define DeclareBuffer(name, type) \
    typedef struct \
    { \
        type * values; \
        size_t count; \
        size_t capacity; \
    } name##Buffer; \
    \
    name##Buffer * name##BufferNew (size_t initial_capacity); \
    void name##BufferFree (name##Buffer * buffer); \
    void name##BufferPush (name##Buffer * buffer, type value); \
    type name##BufferPop  (name##Buffer * buffer)

#define DeclareTable(name, key_type, value_type) \
    typedef struct \
    { \
        key_type key; \
        uint32_t hash; \
        value_type value; \
    } name##Node; \
    \
    typedef struct \
    { \
        name##Node * nodes; \
        size_t count; \
        size_t capacity; \
    } name##Table; \
    \
    name##Table * name##TableNew (size_t initial_capacity); \
    void name##TableFree (name##Table * table); \
    void name##TableAdd (name##Table * table, key_type key, uint32_t hash, value_type value); \
    void name##TableRemove (name##Table * table, uint32_t hash)

DeclareBuffer(Instruction, Instruction);
DeclareBuffer(Value, double);

DeclareTable(Symbol, char *, uint32_t);

char * ReadFile (const char * filename);

void PrintInstructions(InstructionBuffer * buffer);

#endif
