#ifndef _TTY_H_
#define _TTY_H_

#include "../../libc/include/types.h"
#include "vga.h"

void tty_init();
void tty_set_color(const enum vga_color fg, const enum vga_color bg);
void tty_putc(const uint16_t c);
void tty_clear_scr();
int tty_puts(const char* const str);
void tty_update_cursor();
void tty_read(char* dest);
void tty_scroll(int n);
void tty_prompt();

#endif
