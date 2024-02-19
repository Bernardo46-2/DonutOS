// Kernel
#include "../include/tty.h"
#include "../include/idt.h"
#include "../include/asm.h"

// LibC
#include "../../libc/include/stdlib.h"
#include "../../libc/include/stdio.h"

// Main Attraction
#include "../include/donut.h"

void init_os() {
    read_consts();
    tty_init();
    heap_init();
    idt_install();
    idt_init();
}

extern void main() {
    init_os();
    // donut();
    printf("%d\n", (int)gdt_code_addr());
    while(1);
}
