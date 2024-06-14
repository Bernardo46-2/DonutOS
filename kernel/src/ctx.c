#include "../include/ctx.h"
#include "../include/paging.h"
#include "../include/asm.h"

#include "../../libc/include/printf.h"

// URGENT:
// need to setup a TSS inside the GDT

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

static size_t curr_pid = 0;
static tcb_t curr_proc;
uint8_t can_switch = 0;

// in switch_to_task.s
extern void switch_to_task(tcb_t* tcb);

static void process_a() {
    int x = 0;
    while(1) {
        x--;
        CRITICAL_SECTION_START;
        printf("x = %d\n", x);
        CRITICAL_SECTION_END;
    }
}

static void process_b() {
    int y = 0;
    while(1) {
        y++;
        CRITICAL_SECTION_START;
        printf("y = %d\n", y);
        CRITICAL_SECTION_END;
    }
}

int spawn_process(size_t pid, void (*fn)()) {
    void* p = alloc_page();
    if(p == NULL) return 1;
    
    const size_t stack_size = sizeof(regs_t)+1;
    size_t* esp = (size_t*)p - stack_size;
    
    // maybe push esp here
    // esp[0] = (size_t)esp;
    // esp[1] = get_gs(); // gs
    // esp[2] = get_fs(); // fs
    // esp[3] = get_es(); // es
    // esp[4] = get_ds(); // ds
    // esp[5] = 0; // edi
    // esp[6] = 0; // esi
    // esp[7] = (size_t)(esp+stack_size); // ebp
    // esp[8] = (size_t)esp; // esp
    // esp[9] = 0; // ebx
    // esp[10] = 0; // edx
    // esp[11] = 0; // ecx
    // esp[12] = 0; // eax
    // esp[13] = 32; // int_no - double check this
    // esp[14] = 0; // err_no
    // esp[15] = (size_t)fn; // eip
    // esp[16] = get_cs(); // cs
    // esp[17] = get_flags(); // eflags - edit to get flags from main thread
    // esp[18] = (size_t)esp; // useresp
    // esp[19] = get_ss(); // ss

    // set this as current process
    curr_proc = (tcb_t) {
        .esp = (size_t)esp,
        .locked = 0,
        .next = NULL,
        .pid = pid,
        .used = 1,
        .regs = (regs_t) {
            .gs = get_gs(),
            .fs = get_fs(),
            .es = get_es(),
            .ds = get_ds(),

            .edi = 0,
            .esi = 0,
            .ebp = (size_t)(esp+stack_size),
            .esp = (size_t)esp,
            .ebx = 0,
            .edx = 0,
            .ecx = 0,
            .eax = 0,

            .int_no = 32, // double check
            .err_no = 0,

            .eip = (size_t)fn,
            .cs = get_cs(),
            .eflags = get_flags(),
            .useresp = (size_t)esp,
            .ss = get_ss(),
        },
    };
    
    return 0;
}

void scheduler(regs_t* rs) {
    if(can_switch) {
        can_switch = 0;
        // set_esp(curr_proc.esp);
        *rs = curr_proc.regs;
    }
}

void __process_test() {
    can_switch = 1;
    spawn_process(curr_pid++, process_b);
}
