#ifndef _ASM_H_
#define _ASM_H_

#include "../../libc/include/types.h"

void cli();
void sti();
void io_wait();
size_t gdt_code_addr();
uint16_t get_if();

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);

#endif
