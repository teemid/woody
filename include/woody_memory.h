#ifndef WOODY_MEMORY_H
#define WOODY_MEMORY_H

#include <stdlib.h>
#include <string.h>

#define wdy_allocate(bytes) malloc(bytes)
#define wdy_zero_allocate(element_size, element_count) calloc(element_size, element_count)

#define wdy_reallocate(old_memory, bytes) realloc(old_memory, bytes)
#define wdy_deallocate(pointer) free(pointer)
#define wdy_zero(memory, bytes) memset((void *)memory, 0, bytes)

#define wdy_copy(source, destination, bytes) memcpy(destination, source, bytes)

#define wdy_allocate_buffer(type, elements) (type *)malloc(sizeof(type) * elements)
#define wdy_resize_buffer(type, old_buffer, new_size) (type *)realloc(old_buffer, sizeof(type) * new_size)

#endif
