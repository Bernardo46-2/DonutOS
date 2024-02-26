#include "../include/stdio.h"
#include "../include/stdlib.h"
#include "../include/string.h"
#include "../../kernel/include/tty.h"
#include "../../kernel/include/vga.h"

#include <stdarg.h>

#define BUFF_SIZE 1024

void putchar(const char c) {
    tty_putc(c);
}

void puts(const char* const str) {
    size_t i = 0;
    while(str[i]) tty_putc(str[i++]);
    tty_putc('\n');
}

int printf(const char* fmt, ...) {
    if(fmt == NULL) return EOF;
    
    char str[BUFF_SIZE];
    char helper_str[100];
    
    const size_t helper_str_len = 100;
    va_list ap;
    va_start(ap, fmt);
    register char* s = str;
    char* tmp = NULL;
    char c;
    int result = 0;

    while((c = *fmt)) {
        if(c != '%') {
            *s = c;
            s++;
        } else {
            c = *(++fmt);
            switch(c) {
            case '%':
                *s = '%';
                s++;
                break;
            case 'i':
            case 'd':
                itoa(va_arg(ap, int), helper_str, helper_str_len, 10);
                strcpy(s, helper_str);
                s += strlen(helper_str);
                break;
            case 'f':
                // just dont use floats ._.
                break;
            case 'c':
                *s = (char)va_arg(ap, int);
                s++;
                break;
            case 's':
                tmp = va_arg(ap, char*);
                strcpy(s, tmp);
                s += strlen(tmp);
                break;
            default:
                *s = c;
                s++;
                break;
            }
        }

        fmt++;
    }

    *s = '\0';
    result = tty_putstr(str);

    va_end(ap);

    return result;
}
