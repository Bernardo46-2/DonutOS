// Kernel stuff
#include "../include/tty.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/timer.h"
#include "../include/kb.h"

// LibC
#include "../../libc/include/stdlib.h"
#include "../../libc/include/stdio.h"

// Donut
#include "../include/donut.h"

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
    tty_prompt();
    while(1);
}
