#include "../include/timer.h"
#include "../include/pit.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/ctx.h"

#define TPS 1000

static volatile size_t ticks = 0;

void timer_handler(regs_t* rs) {
    ticks++;
    scheduler(rs);
}

inline size_t timer_get() {
    return ticks;
}

void timer_init() {
    pit_set(TPS);
    irq_install(0, timer_handler);
}
