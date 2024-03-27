// Donut
#include "../include/donut.h"

// Kernel stuff
#include "../include/tty.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/timer.h"
#include "../include/kb.h"

// LibC
#include "../../libc/include/malloc.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/rand.h"

void init_os() {
    // os stuff
    tty_init();
    heap_init();
    idt_init();
    
    // hardware stuff
    pic_init();
    isr_init();
    irq_init();
    timer_init();
    kb_init();
}

extern void main() {
    init_os();
    
    donut();
    tty_clear_scr();
    // mrand(timer_get() / (kb_last_key() | 0x1));
    srand(timer_get() * kb_last_key());
    int x, y;
    
     __asm__ __volatile__ (
        "divl %3"  // Divide EDX:EAX pelo divisor
        : "=a" (x), "=d" (y)  // Saídas
        : "a" (0), "r" (0), "d" (0)  // Entradas
    );
    tty_prompt();
    while(1);
}
