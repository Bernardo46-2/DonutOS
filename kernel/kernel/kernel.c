// Kernel stuff
#include "../include/tty.h"
#include "../include/idt.h"
#include "../include/asm.h"
#include "../include/pic.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/timer.h"

// LibC
#include "../../libc/include/stdlib.h"
#include "../../libc/include/stdio.h"

// Donut
#include "../include/donut.h"

void init_os() {
    // os stuff
    read_consts();
    tty_init();
    heap_init();
    
    // hardware stuff
    pic_init();
    idt_init();
    isr_init();
    irq_init();
    timer_init();
}

extern void main() {
    init_os();
    // donut();

    while(1) {
        printf("\n");
        printf("\x1b[1A%d", (int)timer_get());
    }
    
    while(1);
}
