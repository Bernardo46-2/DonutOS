#ifndef _IRQ_H_
#define _IRQ_H_

#include "../../libc/include/types.h"
#include "../include/isr.h"

void irq_install(size_t i, void (*handler)(regs_t*));
void irq_init();

#endif
