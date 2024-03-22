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
    uint8_t packet[4] = {0x13, 0x37, 0xd4, 0x73};
    uint8_t received_packet[virtio_net.rx.desc_size][FRAME_SIZE];

    while (1) {
        printf("Sending packet\n");

        
        
        if (virtio_receive_frame(received_packet, 2048) == 0) {
            printf("Received packet: %x %x %x %x\n", received_packet[0], received_packet[1], received_packet[2], received_packet[3]);
        }
        milisleep(1000);
        //virtio_receive_frame(packet, 4);
    }

    tty_prompt();
    while(1);
}
