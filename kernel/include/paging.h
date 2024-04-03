#ifndef _PAGING_H_
#define _PAGING_H_

#include "../../libc/include/types.h"

#define PAGES_ADDR     0x00100000
#define PAGES_END_ADDR 0x00f00000
#define PAGE_SIZE      0x1000
#define MAX_PAGES      ((PAGES_END_ADDR - PAGES_ADDR) / PAGE_SIZE)

void paging_init();
uint8_t* alloc_page(uint8_t user);
void free_page(size_t paddr);

#endif
