#include "../include/ctx.h"
#include "../include/paging.h"
#include "../include/asm.h"
#include "../include/timer.h"

#include "../../libc/include/printf.h"
#include "../../libc/include/malloc.h"
#include "../../libc/include/string.h"

// #define CTX_DEBUG

#define QUANTUM 1000

static volatile size_t next_pid = 0;
volatile uint8_t can_switch = 0;
volatile uint8_t first_switch = 1;

tcb_t* fst_proc = NULL;
tcb_t* lst_proc = NULL;
tcb_t* curr_proc = NULL;

static void print_regs(size_t pid, regs_t* rs) {
    printf("pid: %d {\n", pid);
    printf("    gs: 0x%08x, fs: 0x%08x, es: 0x%08x, ds: 0x%08x,\n", rs->gs, rs->fs, rs->es, rs->ds);
    printf("    edi: 0x%08x, esi: 0x%08x, ebp: 0x%08x, esp: 0x%08x,\n", rs->edi, rs-> esi, rs->ebp, rs->esp);
    printf("    ebx: 0x%08x, edx: 0x%08x, ecx: 0x%08x, eax: 0x%08x,\n", rs->ebx, rs->edx, rs->ecx, rs->eax);
    printf("    int_no: 0x%08x, err_no: 0x%08x, eip: 0x%08x, cs: 0x%08x,\n", rs->int_no, rs->err_no, rs->eip, rs->cs);
    printf("    eflags: 0x%08x, useresp: 0x%08x, ss: 0x%08x\n", rs->eflags, rs->useresp, rs->ss); 
    printf("}\n\n");
}

static void process_x() {
    int x = 0;
    printf("------------------------------------\n");
    while(1) {
        x++;
#ifndef CTX_DEBUG
        if(x % 50000000 == 0) {
            printf("x = %d timer = %d\n", x, timer_get());
        }
#endif
    }
}

static void process_y() {
    int y = 0;
    printf("------------------------------------\n");
    while(1) {
        y++;
#ifndef CTX_DEBUG
        if(y % 50000000 == 0) {
            printf("y = %d timer = %d\n", y, timer_get());
        }
#endif
    }
}

static void __stub() {
    curr_proc->fn();
    curr_proc->dead = 1;
    while(1);
}

static void __free_ctx(tcb_t* tcb) {
    // TODO
}

int spawn_process(regs_t* rs, void (*fn)(), size_t n_pages) {
    void* p = alloc_pages(n_pages);
    if(p == NULL) return 1;
    
    const size_t stack_size = sizeof(regs_t)+4;
    size_t* esp = (size_t*)p + (PAGE_SIZE * n_pages) - stack_size;
    tcb_t* new_proc = (tcb_t*)malloc(sizeof(tcb_t));

    *new_proc = (tcb_t) {
        .pid = next_pid++,
        .dead = 0,
        .fn = fn,
        
        .n_pages = n_pages,
        .fst_page = p,
        
        .next = NULL,
        
        .regs = (regs_t) {
            .gs = rs->gs,
            .fs = rs->fs,
            .es = rs->es,
            .ds = rs->ds,

            .edi = 0,
            .esi = 0,
            .ebp = (size_t)(esp+stack_size),
            .esp = (size_t)esp,
            .ebx = 0,
            .edx = 0,
            .ecx = 0,
            .eax = 0,

            .int_no = rs->int_no,
            .err_no = rs->err_no,

            .eip = (size_t)__stub,
            .cs = rs->cs,
            .eflags = rs->eflags,
            .useresp = (size_t)esp,
            .ss = rs->ss,
        },
    };

    if(fst_proc == NULL) {
        fst_proc = new_proc;
        lst_proc = new_proc;
        curr_proc = new_proc;

        curr_proc->next = curr_proc;
        curr_proc->prev = curr_proc;
    } else {
        new_proc->prev = lst_proc;
        lst_proc->next = new_proc;
        lst_proc = new_proc;
        
        new_proc->next = fst_proc;
        fst_proc->prev = new_proc;
    }
    
    return 0;
}

static void switch_ctx(regs_t* rs) {
#ifdef CTX_DEBUG
    print_regs(curr_proc->pid, rs);
#endif
    
    // curr_proc->regs = *rs;
    if(!first_switch) {
        first_switch = 0;
        memcpy(&curr_proc->regs, rs, sizeof(regs_t));
    }
    curr_proc = curr_proc->next;
    // *rs = curr_proc->regs;
    memcpy(rs, &curr_proc->regs, sizeof(regs_t));
    
#ifdef CTX_DEBUG
    print_regs(curr_proc->pid, rs);
#endif
}

void scheduler(regs_t* rs) {
    static size_t last_ticks = 0;
    size_t now = timer_get();

    if(now > last_ticks + QUANTUM) {
        last_ticks = now;

        // TODO: handle dead processes
        if(can_switch && curr_proc->next != curr_proc) {
            switch_ctx(rs);
        }
    }
}

void __spawn_dummy_processes(regs_t *rs) {
    uint8_t x = spawn_process(rs, process_x, 4);
    uint8_t y = spawn_process(rs, process_y, 4);
    printf("%s\n", x || y ? "some error occured" : "dummy processes spawned");

#ifdef CTX_DEBUG
    printf("kernel regs:\n");
    print_regs(0, rs);
    
    tcb_t* ptr = NULL;
    for(ptr = fst_proc; ptr != lst_proc; ptr = ptr->next) {
        print_regs(ptr->pid, &ptr->regs);
    }
    print_regs(ptr->pid, &ptr->regs);
#endif
}

void __proc_kb_debug(regs_t* rs, unsigned char key) {
    static uint8_t fst_press = 1;
    
    if(fst_press) {
        fst_press = 0;
        __spawn_dummy_processes(rs);
    } else {
        can_switch = !can_switch;
    }
}

// TODO: create kernel process
void __process_test() {
    curr_proc = lst_proc;
    can_switch = 1;
}
