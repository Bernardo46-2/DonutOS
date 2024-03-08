#include "../include/isr.h"
#include "../include/idt.h"
#include "../include/blue_scr.h"

#define NUM_ISRS 48

// these are all in isr_wrapper.s
extern void _isr0(regs_t* rs);
extern void _isr1(regs_t* rs);
extern void _isr2(regs_t* rs);
extern void _isr3(regs_t* rs);
extern void _isr4(regs_t* rs);
extern void _isr5(regs_t* rs);
extern void _isr6(regs_t* rs);
extern void _isr7(regs_t* rs);
extern void _isr8(regs_t* rs);
extern void _isr9(regs_t* rs);
extern void _isr10(regs_t* rs);
extern void _isr11(regs_t* rs);
extern void _isr12(regs_t* rs);
extern void _isr13(regs_t* rs);
extern void _isr14(regs_t* rs);
extern void _isr15(regs_t* rs);
extern void _isr16(regs_t* rs);
extern void _isr17(regs_t* rs);
extern void _isr18(regs_t* rs);
extern void _isr19(regs_t* rs);
extern void _isr20(regs_t* rs);
extern void _isr21(regs_t* rs);
extern void _isr22(regs_t* rs);
extern void _isr23(regs_t* rs);
extern void _isr24(regs_t* rs);
extern void _isr25(regs_t* rs);
extern void _isr26(regs_t* rs);
extern void _isr27(regs_t* rs);
extern void _isr28(regs_t* rs);
extern void _isr29(regs_t* rs);
extern void _isr30(regs_t* rs);
extern void _isr31(regs_t* rs);
extern void _isr32(regs_t* rs);
extern void _isr33(regs_t* rs);
extern void _isr34(regs_t* rs);
extern void _isr35(regs_t* rs);
extern void _isr36(regs_t* rs);
extern void _isr37(regs_t* rs);
extern void _isr38(regs_t* rs);
extern void _isr39(regs_t* rs);
extern void _isr40(regs_t* rs);
extern void _isr41(regs_t* rs);
extern void _isr42(regs_t* rs);
extern void _isr43(regs_t* rs);
extern void _isr44(regs_t* rs);
extern void _isr45(regs_t* rs);
extern void _isr46(regs_t* rs);
extern void _isr47(regs_t* rs);

static void (*isrs[NUM_ISRS])(regs_t* rs) = {
    _isr0,  _isr1,  _isr2,  _isr3,  _isr4,  _isr5,  _isr6,  _isr7,  _isr8,  _isr9, _isr10, _isr11,
    _isr12, _isr13, _isr14, _isr15, _isr16, _isr17, _isr18, _isr19, _isr20, _isr21, _isr22, _isr23,
    _isr24, _isr25, _isr26, _isr27, _isr28, _isr29, _isr30, _isr31, _isr32, _isr33, _isr34, _isr35,
    _isr36, _isr37, _isr38, _isr39, _isr40, _isr41, _isr42, _isr43, _isr44, _isr45, _isr46, _isr47,
};

static const char* exception_msgs[33] = {
    "Division Error",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "RESERVED",
    "x86 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "Hypervision Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "RESERVED",
    "this should never be printed"
};

static void (*handlers[NUM_ISRS])(regs_t* rs) = { 0 };

inline void isr_install(size_t index, void (*handler)(regs_t* rs)) {
    handlers[index] = handler;
}

// called from isr_wrapper.s
void isr_handler(regs_t* rs) {
    if(handlers[rs->int_no]) {
        handlers[rs->int_no](rs);
    }
}

static void __exception_handler(regs_t* rs) {
    blue_scr(rs->int_no, exception_msgs[rs->int_no]);
    while(1);
}

void isr_init() {
    for(size_t i = 0; i < NUM_ISRS; i++)
        idt_set_gate(i, isrs[i], 0x08, 0x8E);

    for(size_t i = 0; i < 32; i++)
        isr_install(i, __exception_handler);
}
