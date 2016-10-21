#ifndef WOODY_MEMORY_H
#define WOODY_MEMORY_H

#include <stdlib.h>

#define Allocate(bytes) malloc(bytes)
#define ZeroAllocate(element_size, element_count) calloc(element_size, element_count)
#define Reallocate(old_memory, bytes) realloc(old_memory, bytes)
#define Deallocate(pointer) free(pointer)
#define Zero(memory, bytes) memset((void *)memory, 0, bytes)

#define Buffer(type, elements) (type *)malloc(sizeof(type) * elements)
#define ResizeBuffer(type, old_buffer, new_size) (type *)realloc(old_buffer, sizeof(type) * new_size)

#endif
