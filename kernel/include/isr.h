#ifndef _ISR_H_
#define _ISR_H_

#include "../../libc/include/types.h"

#pragma pack(1)
typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_no;
    uint32_t eip, cs, eflags;
} regs_t;
#pragma pack()

void isr_install(size_t index, void (*handler)(regs_t* rs));
void isr_init();

#endif
