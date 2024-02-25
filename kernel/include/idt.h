#ifndef _IDT_H_
#define _IDT_H_

#include "../../libc/include/types.h"
#include "isr.h"

void idt_init();
void idt_set_gate(uint8_t index, void (*base)(regs_t* rs), uint16_t sel, uint8_t flags);

#endif
