#ifndef _TTY_H_
#define _TTY_H_

#include "../../libc/include/types.h"
#include "vga.h"

void tty_init();
void tty_set_color(const enum vga_color fg, const enum vga_color bg);
void tty_putc(const uint16_t c);
int tty_putstr(char* str);

#endif
