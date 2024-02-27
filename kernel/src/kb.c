#include "../include/kb.h"
#include "../include/asm.h"
#include "../include/isr.h"
#include "../include/irq.h"

// TODO: remove this
#include "../../libc/include/stdio.h"

static void __kb_handler(regs_t* rs) {
    uint16_t scancode = inb(0x60);
    printf("\n\033[1A%d", scancode);
}

void kb_init() {
    irq_install(1, __kb_handler);
}
