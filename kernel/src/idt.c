#include "../include/idt.h"
#include "../../libc/include/string.h"

// this is defined in DonutOS/kernel/src/idt_loader.s
extern void load_idt();

#pragma pack(1)
typedef struct {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_hi;
} idt_entry_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint16_t limit;
    size_t base;
} idt_ptr_t;
#pragma pack()

idt_entry_t idt[256];
idt_ptr_t idtp;

void idt_install() {
    idtp.base = (size_t)(&idt);
    idtp.limit = sizeof idt - 1;
    memset(&idt, 0, sizeof idt);
    load_idt();
} 

void idt_init() {
    load_idt();
}

void idt_set_gate(uint8_t index, size_t base, uint16_t sel, uint8_t flags) {
    idt[index].base_lo = base & 0xffff;
    idt[index].sel = sel;
    idt[index].zero = 0;
    idt[index].flags = flags;
    idt[index].base_hi = base >> 16;
}

uint8_t idt_entry_create_flags(uint8_t gate_type, uint8_t dpl, uint8_t present) {
    uint8_t flags = 0;
    // TODO
    return flags;
}
