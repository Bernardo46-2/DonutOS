#include "../include/ctx.h"
#include "../include/paging.h"
#include "../include/asm.h"

#include "../../libc/include/printf.h"
#include "../../libc/include/malloc.h"

// each context should have its own stack
// keep an extra stack segment for kernel (probably the same stack currently being used)

// -- CONTEXT SWITCH --
// when an interrupt triggers, the context will be pushed onto the stack
// pop that stuff and store it in process_list (probably can just copy and then edit the values on the regs_t* struct)
// search for next context to switch to
// fill the stack with new context
// let interrupt play as usuall

// if a context is storing packages to a buffer, treat that as a critical section
// do not switch to a different context then

static ctx_t process_list[NUM_PROCESSES] = { 0 };
static size_t curr_pid = 0;
static uint8_t can_switch = 1;

#define CRITICAL_SECTION_START (can_switch = 0)
#define CRITICAL_SECTION_END   (can_switch = 1)

static void process_a() {
    int x = 0;
    while(1) {
        x--;
        CRITICAL_SECTION_START;
        printf("x = %d ", x);
        CRITICAL_SECTION_END;
    }
}

static void process_b() {
    int y = 0;
    while(1) {
        y++;
        CRITICAL_SECTION_START;
        printf("y = %d ", y);
        CRITICAL_SECTION_END;
    }
}

int spawn_process(size_t pid, void (*function)()) {
    if(process_list[pid].used == 1) return 1;

    uint8_t* page = alloc_page();
    
    process_list[pid] = (ctx_t) {
        .pid = pid,
        .used = 1,
        .locked = 0,
        // .regs = FIGURE THIS OUT;
    };
    
    return 0;
}

void round_robin() {
    can_switch = 1;
    while(1) {
        // TODO
    }
}

void process_test() {
    spawn_process(0, process_a);
    spawn_process(1, process_b);
    round_robin();
}
