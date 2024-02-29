#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "types.h"

#define HEAP_ADDRESS 0x50000
#define STACK_ADDRESS 0x90000

extern const uint32_t HEAP_MAX_SIZE;

void heap_init();

void* malloc(size_t size);
void* calloc(size_t size, size_t reg_size);
void free(void* ptr);

#endif
