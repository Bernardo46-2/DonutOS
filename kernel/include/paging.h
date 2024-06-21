#ifndef _PAGING_H_
#define _PAGING_H_

#include "../../libc/include/types.h"

#define PAGE_DIR_SIZE       1024
#define PAGE_TABLE_SIZE     1024
#define PAGE_DIR_ADDR       0x00300000
#define PAGE_TABLE_ADDR     (PAGE_DIR_ADDR + 0x1000)
#define PAGE_SIZE           0x1000 // 4 kb
#define PAGE_TABLES_COUNT   (TOTAL_MEMORY_SIZE / (PAGE_SIZE * PAGE_TABLE_SIZE))

#define USER_PAGES_ADDR     (PAGE_DIR_ADDR + PAGE_TABLES_COUNT * PAGE_SIZE)

#define TOTAL_MEMORY_SIZE   (128 * 1024 * 1024) // 128 mb
#define TOTAL_PAGES         (TOTAL_MEMORY_SIZE / PAGE_SIZE)

void paging_init();
void* alloc_page();
void* alloc_pages(size_t n);
void free_page(void* page);
void free_pages(void* page, size_t n);
void __paging_test();

#endif
