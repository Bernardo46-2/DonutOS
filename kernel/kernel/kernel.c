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
#include "../include/virtio_net.h"
#include "../include/sys.h"

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
    
    int err = virtio_net_init();
    if (err) printf("Error %d, while trying to start the network device: ", err);
    switch (err)
    {
    case 0:
        break;
    case ERR_DEVICE_BAD_CONFIGURATION:
        printf("ERR_DEVICE_BAD_CONFIGURATION\n");
        break;
    case ERR_CONFIG_NOT_ACCEPTED:
        printf("ERR_CONFIG_NOT_ACCEPTED\n");
        break;
    case ERR_DEVICE_NOT_FOUND:
        printf("ERR_DEVICE_NOT_FOUND\n");
        break;
    }
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

    tty_clear_scr();
    tty_prompt();
    while(1);
}
