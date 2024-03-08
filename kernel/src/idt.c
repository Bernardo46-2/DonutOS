#include "../include/idt.h"
#include "../../libc/include/string.h"

// in DonutOS/kernel/src/idt_loader.s
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

idt_entry_t idt_entries[256];
idt_ptr_t idtp;

void idt_install() {
    idtp.base = (size_t)(&idt_entries);
    idtp.limit = sizeof idt_entries - 1;
    memset(&idt_entries, 0, sizeof idt_entries);
} 

void idt_init() {
    idt_install();
    load_idt();
}

void idt_set_gate(uint8_t index, void (*base)(regs_t* rs), uint16_t sel, uint8_t flags) {
    idt_entries[index] = (idt_entry_t) {
        .base_lo = ((size_t)base) & 0xffff,
        .sel = sel,
        .zero = 0,
        .flags = flags,
        .base_hi = ((size_t)base) >> 16 & 0xffff
    };
}
