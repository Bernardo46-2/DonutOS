// Kernel stuff
#include "../include/tty.h"
#include "../include/idt.h"
#include "../include/asm.h"
#include "../include/pic.h"

// LibC
#include "../../libc/include/stdlib.h"
#include "../../libc/include/stdio.h"

// Main Attraction
#include "../include/donut.h"

void init_os() {
    read_consts();
    pic_init();
    idt_init();
    tty_init();
    heap_init();
}

extern void main() {
    init_os();
    donut();
    while(1);
}
