#ifndef _CTX_H_
#define _CTX_H_

#include "../../libc/include/types.h"
#include "../include/isr.h"

#define CRITICAL_SECTION_START (can_switch = 0)
#define CRITICAL_SECTION_END   (can_switch = 1)

#pragma pack(1)
typedef struct tcb_s {
    size_t pid;
    uint8_t dead;
    void (*fn)();
    
    size_t n_pages;
    void* fst_page;
    
    struct tcb_s* prev;
    struct tcb_s* next;
    regs_t regs;
} tcb_t;
#pragma pack()

extern volatile uint8_t can_switch;

int spawn_process(regs_t* rs, void (*fn)(), size_t n_pages);
void scheduler(regs_t* rs);
void __spawn_dummy_processes(regs_t* rs);
void __proc_kb_debug(regs_t* rs, unsigned char key);
void __process_test();

#endif
