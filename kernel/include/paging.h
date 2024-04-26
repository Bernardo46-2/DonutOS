#ifndef _PAGING_H_
#define _PAGING_H_

#include "../../libc/include/types.h"

#define PAGE_DIR_SIZE       1024
#define PAGE_DIR_ADDR       0x00100000
#define PAGE_TABLE_ADDR     (PAGE_DIR_ADDR + 0x1000)
#define PAGE_TABLE_SIZE     1024
#define PAGE_SIZE           0x1000 // 4 kb

void paging_init();
void alloc_page();
void paging_test();

#endif
