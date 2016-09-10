#ifndef WOODY_MEMORY_H
#define WOODY_MEMORY_H

#define Allocate(bytes) malloc(bytes)
#define Reallocate(old_memory, bytes) realloc(old_memory, bytes)
#define Deallocate(pointer) free(pointer)

#define Buffer(type, elements) (type *)malloc(sizeof(type) * elements)
#define ReallocateBuffer(type, old_buffer, new_size) (type *)realloc(old_buffer, sizeof(type) * new_size)

#endif
