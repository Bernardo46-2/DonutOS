#include "../include/paging.h"
#include "../include/asm.h"

#include "../../libc/include/printf.h"

static size_t* const page_directory = (size_t*)PAGE_DIR_ADDR;

static void paging_setup() {
    for(size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        page_directory[i] = 0x2; // supervisor, read/write, not present
    }
}

void paging_init() {
    set_cr3((size_t)page_directory);
    set_cr0(get_cr0() | (1 << 31));
}

// TODO: write a propper page allocator
void alloc_page() {
    size_t* page_table = (size_t*)PAGE_TABLE_ADDR;

    for(size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i] = (i * 0x1000) | 3;
    }
    
    page_directory[0] = ((size_t)page_table) | 3;
}

void __paging_test() {
    paging_setup();
    alloc_page();
    paging_init();
    printf("We're pagin' babyyy\n");
}
