#include "../include/paging.h"
#include "../include/asm.h"

#include "../../libc/include/printf.h"

static size_t* const page_directory = (size_t*)PAGE_DIR_ADDR;
static size_t page_bitmap[TOTAL_PAGES / 32];

static void* const user_pages = (uint8_t*)USER_PAGES_ADDR;
static const size_t init_idx = USER_PAGES_ADDR / PAGE_SIZE;

static void __alloc_page(size_t bit) {
    page_bitmap[bit >> 5] |= (1 << (bit & 31));
}

static void __free_page(size_t bit) {
    page_bitmap[bit >> 5] &= ~(1 << (bit & 31));
}

static uint8_t __is_page_free(size_t bit) {
    return !(page_bitmap[bit >> 5] & (1 << (bit & 31)));
}

static void paging_setup() {
    for(size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        page_directory[i] = 0x2; // supervisor, read/write, not present
    }
    
    // mapping out all the 128 mb of ram the os uses
    for(size_t table_index = 0; table_index < PAGE_TABLES_COUNT; table_index++) {
        size_t* page_table = (size_t*)(PAGE_TABLE_ADDR + table_index * PAGE_SIZE);
        
        for(size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
            page_table[i] = (table_index * PAGE_TABLE_SIZE * PAGE_SIZE + i * PAGE_SIZE) | 3;
        }
        
        page_directory[table_index] = ((size_t)page_table) | 3;
    }

    for(size_t i = 0; i < TOTAL_PAGES / PAGE_SIZE; i++) {
        page_bitmap[i] = 0x0;
    }

    for(size_t i = 0; i < (USER_PAGES_ADDR / PAGE_SIZE); i++) {
        __alloc_page(i);
    }
}

void paging_init() {
    paging_setup();
    set_cr3((size_t)page_directory);
    set_cr0(get_cr0() | (1 << 31));
}

void* alloc_page() {
    for(size_t i = init_idx; i < TOTAL_PAGES; i++) {
        if(__is_page_free(i)) {
            __alloc_page(i);
            return (void*)(i << 12);
        }
    }
    return NULL;
}

// allocates continuous pages
void* alloc_pages(size_t n) {
    uint8_t b = 1;
    
    for(size_t i = init_idx; i < TOTAL_PAGES; i++) {
        if(__is_page_free(i) && __is_page_free(i+n-1)) {
            b = 1;
            
            for(size_t j = i+1; j < TOTAL_PAGES && j < i+n; j++) {
                if(!__is_page_free(j)) {
                    j = TOTAL_PAGES; // break
                    b = 0;
                }
            }

            if(b) {
                for(size_t j = i; j < i+n; j++) {
                    __alloc_page(j);
                }
                return (void*)(i << 12);
            }
        }
    }
    
    return NULL;
}

void free_page(void* page) {
    if(page < user_pages) return;
    __free_page((size_t)page >> 12);
}

// frees continuos pages
void free_pages(void* page, size_t n) {
    if(page < user_pages) return;
    
    size_t idx = (size_t)page >> 12;
    for(size_t i = idx; i < idx+n; i++) {
        __free_page(i);
    }
}

void __paging_test() {
#ifdef PAGING_DEBUG
    void* a = alloc_page();
    void* b = alloc_page();
    void* c = alloc_page();
    void* d = alloc_page();

    free_page(b);
    void* e = alloc_pages(2);
    free_pages(e, 2);
#endif

    printf("User pages allocated:\n");
    printf("=======================\n");
    for(size_t i = USER_PAGES_ADDR / PAGE_SIZE; i < TOTAL_PAGES; i++) {
        if(!__is_page_free(i)) {
            printf("Page %d: 0x%08x\n", (uint32_t)i, (uint32_t)(i << 12));
        }
    }
    printf("=======================\n\n");
}
