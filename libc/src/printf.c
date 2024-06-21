#include "../include/printf.h"
#include "../include/stdio.h"
#include "../include/stdlib.h"
#include "../include/string.h"

#include "../../kernel/include/vga.h"
#include "../../kernel/include/tty.h"

#include <stdarg.h>

static void str_to_upper(char *str) {
    while(*str) {
        if(*str >= 'a' && *str <= 'z') {
            *str = *str - 'a' + 'A';
        }
        str++;
    }
}

int printf(const char* fmt, ...) {
    if(fmt == NULL) return EOF;
    
    const size_t helper_len = 128;
    
    char str[VGA_SCR_SIZE];
    char helper_str[helper_len];
    
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

            // handling stuff like "%09d", in which there should be
            // 9 chars, prefixed with a 0 padding
            char leading_char = ' ';
            size_t leading_len = 0;
            size_t value_len = 0;
            
            if(c == '0') {
                leading_char = '0';
                fmt++;
            }
            
            size_t i = 0;
            c = *fmt;
            while('0' <= c && c <= '9') {
                helper_str[i++] = c;
                c = *(++fmt);
            }

            if(i < helper_len) helper_str[i] = '\0';
            if(helper_str[0] != '\0') leading_len = atoi(helper_str);
            
            // handling the actual formatting type
            switch(c) {
                case '%':
                    *s = '%';
                    s++;
                    break;
                case 'i':
                case 'd':
                    itoa(va_arg(ap, int), helper_str, helper_len, 10);
                    
                    value_len = strlen(helper_str);
                    if(leading_len > value_len)
                        memset(s, leading_char, leading_len - value_len);
                    
                    if(leading_len > value_len)
                        s += leading_len - value_len;
                    strcpy(s, helper_str);
                    s += value_len;
                    break;
                case 'x':
                    itoa(va_arg(ap, int), helper_str, helper_len, 16);
                    
                    value_len = strlen(helper_str);
                    if(leading_len > value_len)
                        memset(s, leading_char, leading_len - value_len);
                    
                    if(leading_len > value_len)
                        s += leading_len - value_len;
                    strcpy(s, helper_str);
                    s += value_len;
                    break;
                case 'X':
                    itoa(va_arg(ap, int), helper_str, helper_len, 16);
                    str_to_upper(helper_str);
                    value_len = strlen(helper_str);
                    if(leading_len > value_len)
                        memset(s, leading_char, leading_len - value_len);
                    
                    if(leading_len > value_len)
                        s += leading_len - value_len;
                    strcpy(s, helper_str);
                    s += value_len;
                    break;
                case 'b':
                    itoa(va_arg(ap, int), helper_str, helper_len, 2);
                    
                    value_len = strlen(helper_str);
                    if(leading_len > value_len)
                        memset(s, leading_char, leading_len - value_len);
                    
                    if(leading_len > value_len)
                        s += leading_len - value_len;
                    strcpy(s, helper_str);
                    s += value_len;
                    break;
                case 'f':
                    {
                        double value = va_arg(ap, double);
                        int int_part = (int)value;
                        double fraction_part = value - int_part;
    
                        itoa(int_part, helper_str, helper_len, 10);
                        value_len = strlen(helper_str);
                        if(leading_len > value_len)
                            memset(s, leading_char, leading_len - value_len);
    
                        if(leading_len > value_len)
                            s += leading_len - value_len;
                        strcpy(s, helper_str);
                        s += value_len;
    
                        // Decimal point
                        *s = '.';
                        s++;
    
                        // Convert fraction part to string
                        for(int i = 0; i < 6; i++) {
                            fraction_part *= 10;
                            int digit = (int)fraction_part;
                            helper_str[i] = '0' + digit;
                            fraction_part -= digit;
                        }
                        helper_str[6] = '\0';
                        value_len = strlen(helper_str);
                        strcpy(s, helper_str);
                        s += value_len;
                    }
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
    result = tty_puts(str);

    va_end(ap);

    return result;
}
