#include "../include/pic.h"
#include "../include/asm.h"

// master pic handles irq < 8
// slave pic handles 8 <= irq < 16

#define PIC1            0x20      // master PIC
#define PIC2            0xa0      // slave PIC
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define PIC2_DATA       (PIC2+1)
#define PIC_EOI         0x20

// Initialization Command Words
#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10

#define ICW4_8086       0x01
#define ICW4_AUTO       0x02
#define ICW4_BUF_SLAVE  0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM       0x10

void pic_send_eoi(uint8_t irq) {
    if(irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_remap(int offset1, int offset2) {
    // initial settings
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // setting up the base offset
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    // linking both pics
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    // setting the pic mode to 8086
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0);
    io_wait();
    outb(PIC2_DATA, 0);
    io_wait();
    outb(PIC1_DATA, 0);
    io_wait();
    outb(PIC2_DATA, 0);
    io_wait();
}

void pic_init() {
    for(size_t i = 0; i < 16; i++)
        pic_set_mask(i);
    pic_remap(0x20, 0x28);
    pic_remap(0x20, 0x28);
    sti();
}

// =============================================================== //

// interrupt masks are done by setting a 1 or 0 to the bit index of 
// same number as that irq_line. e.g. to disable interrupt 4, set a 
// 1 on the bit of index 4 of the Interrupt Mask Register (IMR)

// disabling irq_line
void pic_set_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    if(irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    value = inb(port) | (1 << irq_line);
    outb(port, value);
}

// enabling irq_line
void pic_clear_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;
    
    if(irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    value = inb(port) & ~(1 << irq_line);
    outb(port, value);
}

// =============================================================== //

#define PIC_READ_ISR    0x0a
#define PIC_READ_IRR    0x0b

static uint16_t __pic_get_irq_reg(int ocw3) {
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t pic_get_isr() {
    return __pic_get_irq_reg(PIC_READ_ISR);
}

uint16_t pic_get_irr() {
    return __pic_get_irq_reg(PIC_READ_IRR);
}
