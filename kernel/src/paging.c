#include "../include/paging.h"

static uint8_t* const pages_addr = (uint8_t*)PAGES_ADDR;

typedef struct {
    uint8_t free : 1;
    uint8_t user : 1;
    size_t paddr : 32;
} page_frame_t;

static page_frame_t pages[MAX_PAGES] = { 0 };

void paging_init() {
    for(size_t i = 0; i < MAX_PAGES; i++) {
        pages[i] = (page_frame_t) {
            .free = 1,
            .user = 0,
            .paddr = PAGES_ADDR + (i * PAGE_SIZE)
        };
    }
}

uint8_t* alloc_page(uint8_t user) {
    for(size_t i = 0; i < MAX_PAGES; i++) {
        if(pages[i].free) {
            pages[i] = (page_frame_t) {
                .free = 0,
                .user = user,
            };
            return pages_addr + (i * PAGE_SIZE);
        }
    }
    return NULL;
}

void free_page(size_t paddr) {
    size_t index = paddr / PAGE_SIZE;
    pages[index].free = 1;
}

