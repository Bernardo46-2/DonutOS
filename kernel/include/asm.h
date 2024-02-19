#ifndef _ASM_H_
#define _ASM_H_

#include "../../libc/include/types.h"

void cli();
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
void read_consts();
size_t gdt_code_addr();

#endif
