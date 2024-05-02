#ifndef _PIC_H_
#define _PIC_H_

#include "../../libc/include/types.h"

void pic_send_eoi(uint8_t irq);
void pic_remap(int offset1, int offset2);
void pic_init();

void pic_set_mask(uint8_t irq_line);
void pic_clear_mask(uint8_t irq_line);

uint16_t pic_get_isr();
uint16_t pic_get_irr();

#endif