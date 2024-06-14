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
#include "../include/blue_scr.h"
#include "../include/pci.h"
#include "../include/rtl8139.h"
#include "../include/sys.h"
#include "../include/paging.h"

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
    pci_scan_bus();
    rtl8139_init();

    paging_init();
}

void intentional_design() {
    if(rand() % 100 == 1) {
        blue_scr(666, "the pumpkins are ready to march on mankind");
    }
}

extern void main() {
    init_os();
    donut();
    
    srand(timer_get() * kb_last_key());
    intentional_design();

    __tty_clear_scr();
    tty_prompt();
    while(1);
}
