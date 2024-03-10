#include "../include/seg.h"

#include "../../libc/include/printf.h"

#define PROCESS_AREA_BASE 0x100000
#define PROCESS_AREA_END 0xefffff
#define PROCESS_AREA_LIMIT (PROCESS_AREA_END - PROCESS_AREA_BASE)

typedef struct {
    size_t base;
    size_t limit;
} seg_table_t;

void seg_test() {
    printf("limit = %08x\n", PROCESS_AREA_LIMIT);
    printf("base+limit = %08x\n", PROCESS_AREA_BASE + PROCESS_AREA_LIMIT);
}

