#ifndef _CTX_H_
#define _CTX_H_

#include "../../libc/include/types.h"
#include "../include/isr.h"

#define NUM_PROCESSES 2
#define CRITICAL_SECTION_START (can_switch = 0)
#define CRITICAL_SECTION_END   (can_switch = 1)

typedef struct tcb_s {
    size_t pid;
    uint8_t used : 1;
    uint8_t locked : 1;
    
    size_t esp;
    struct tcb_s* next;
    regs_t regs;
} tcb_t;

extern uint8_t can_switch;

int spawn_process(size_t pid, void (*fn)());
void scheduler(regs_t* rs);
void __process_test();

#endif
