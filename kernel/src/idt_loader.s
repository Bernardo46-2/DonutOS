[bits 32]
extern idtp
global load_idt
load_idt:
    lidt [idtp]
    ret
