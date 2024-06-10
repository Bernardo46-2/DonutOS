#include "../include/malloc.h"

// well, here's a malloc implementation, Alexandre. Rest in peace dude

const uint32_t HEAP_MAX_SIZE = STACK_ADDRESS - HEAP_ADDRESS;
const size_t TOTAL_MEMORY = HEAP_MAX_SIZE;
uint8_t* const HEAP_PTR = (uint8_t*)HEAP_ADDRESS;
uint8_t* const STACK_BASE_PTR = (uint8_t*)STACK_ADDRESS;

size_t memory_used = 0;

#pragma pack(1)
typedef struct {
    uint8_t is_free : 1;
    uint32_t size : 31;
} AllocHeader;
#pragma pack()

static inline uint8_t __alloc_is_free(const uint8_t* const ptr) {
    return ((AllocHeader*)ptr)->is_free;
}

static inline uint32_t __alloc_size(const uint8_t* const ptr) {
    return ((AllocHeader*)ptr)->size;
}

void heap_init() {
    AllocHeader* p = (AllocHeader*)HEAP_PTR;
    p->is_free = 1;
    p->size = HEAP_MAX_SIZE;
}

static uint8_t* __malloc_find_empty_space(uint8_t* p) {
    while(p < STACK_BASE_PTR && !__alloc_is_free(p)) {
        p += __alloc_size(p);
    }
    return p;
}

static uint8_t* __malloc_concat_free_chunks(uint8_t* p) {
    if(p < STACK_BASE_PTR && __alloc_is_free(p)) {
        AllocHeader* alloc_p = (AllocHeader*)p;
        uint8_t* concat_ptr = p + alloc_p->size;
        
        while(concat_ptr < STACK_BASE_PTR && __alloc_is_free(concat_ptr)) {
            uint32_t concat_size = __alloc_size(concat_ptr);
            alloc_p->size += concat_size;
            concat_ptr += concat_size;
        }
    }
    return p;
}

void* malloc(size_t size) {
    size += sizeof(AllocHeader);
    if(size >= HEAP_MAX_SIZE || size == 0) return NULL;
    
    uint8_t* p = HEAP_PTR;
    uint8_t alloc_successful = 0;

    while(p < STACK_BASE_PTR && !alloc_successful) {
        p = __malloc_find_empty_space(p);
        p = __malloc_concat_free_chunks(p);

        if(p < STACK_BASE_PTR && __alloc_is_free(p)) {
            if(size < __alloc_size(p)) {
                AllocHeader* header = (AllocHeader*)p;
                uint32_t next_size = header->size - size;

                header->is_free = 0;
                header->size = size;
                memory_used += header->size;
                
                
                if(next_size > sizeof(AllocHeader)) {
                    //Next header
                    header = (AllocHeader*)(p + size);
                    header->is_free = 1;
                    header->size = next_size;
                } else {
                    header->size += next_size;
                }
                

                alloc_successful = 1;
            } else {
                p += __alloc_size(p);
            }
        }
    }
    
    
    return alloc_successful ? (void*)(p+sizeof(AllocHeader)) : NULL;
}

void* calloc(const size_t size, const size_t reg_size) {
    const size_t bytes = size * reg_size;
    uint8_t* array = (uint8_t*)malloc(bytes);
    for(size_t i = 0; i < bytes; i++) array[i] = 0;
    return (void*)array;
}

void free(void* ptr) {
    AllocHeader* p = (AllocHeader*)ptr;
    (--p)->is_free = 1;
    memory_used -= p->size;
}
