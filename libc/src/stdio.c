#include "../include/stdio.h"
#include "../include/stdlib.h"
#include "../include/string.h"
#include "../../kernel/include/tty.h"
#include "../../kernel/include/vga.h"

#include <stdarg.h>

void putchar(const char c) {
    tty_putc(c);
}

void puts(const char* const str) {
    tty_puts(str);
    tty_putc('\n');
}

