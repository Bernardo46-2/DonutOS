#ifndef _CTX_H_
#define _CTX_H_

#include "../../libc/include/types.h"
#include "../include/isr.h"

#define NUM_PROCESSES 2

typedef struct tcb_s {
    size_t pid;
    uint8_t used : 1;
    uint8_t locked : 1;
    
    size_t esp;
    struct tcb_s* next;
} tcb_t;

#endif
