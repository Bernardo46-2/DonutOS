#include "../include/pit.h"
#include "../include/idt.h"
#include "../include/asm.h"

#define PIT_IRQ 0

// 1000 Hz == 1ms
#define PIT_FREQ 1000

void pit_interrupt_handler() {
    // TODO
} 

void pit_init() {
    idt_set_gate(0x0, (size_t)pit_interrupt_handler, (uint16_t)gdt_code_addr(), 0);
}
