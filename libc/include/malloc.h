#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "types.h"

#define HEAP_ADDRESS 0x00100000
#define STACK_ADDRESS 0x00f00000

extern const size_t TOTAL_MEMORY;
extern const uint32_t HEAP_MAX_SIZE;
extern size_t memory_used;

void heap_init();
void* malloc(size_t size);
void* calloc(size_t size, size_t reg_size);
void free(void* ptr);

#endif
