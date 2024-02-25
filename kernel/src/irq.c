#include "../include/irq.h"
#include "../include/asm.h"
#include "../include/pic.h"
#include "../include/isr.h"

#define NUM_IRQS 16

static void (*handlers[NUM_IRQS])(regs_t* rs) = { 0 };

static void __stub(regs_t* rs) {
    if(32 <= rs->int_no && rs->int_no <= 47) {
        if(handlers[rs->int_no - 32]) {
            handlers[rs->int_no - 32](rs);
        }
    }
    
    pic_send_eoi(rs->int_no - 32);
}

void irq_install(size_t i, void (*handler)(regs_t*)) {
    cli();
    handlers[i] = handler;
    pic_clear_mask(i);
    sti();
}

// assumes PIC is already setup
void irq_init() {
    for(size_t i = 0; i < NUM_IRQS; i++)
        isr_install(i+32, __stub);
}
