#include "../include/tty.h"
#include "../../libc/include/stdlib.h"
#include "../../libc/include/stdio.h"

#include "../include/donut.h"

void init_os() {
    tty_init();
    heap_init();
}

extern void main() {
    init_os();
    donut();
    
    while(1);
}
