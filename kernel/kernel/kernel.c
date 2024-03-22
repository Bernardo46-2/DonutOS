// Kernel stuff
#include "../include/tty.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/timer.h"
#include "../include/kb.h"
#include "../include/virtio_net.h"


// LibC
#include "../../libc/include/malloc.h"
#include "../../libc/include/printf.h"
#include "../../libc/include/time.h"

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

    //Test

    pci_scan_bus();
    virtio_net_init();

    while (1) {
        printf("Waiting for frame...\n");
        
        virtio_receive_frame();
        milisleep(1000);
    }

    tty_prompt();
    while(1);
}
