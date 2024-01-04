#include "../include/stdlib.h"
#include "../include/string.h"

int itoa(int num, char* str, int len, int base) {
    if(len == 0) return -1;
    
    size_t i = 0;
    int digit;
    
    do {
        digit = num % base;

        if(digit < 0xa) str[i++] = '0' + digit;
        else str[i++] = 'A' + digit - 0xa;

        num /= base;
    } while(num && (i < (len-1)));

    if(i == (len-1) && num) return -1;

    str[i] = '\0';
    strrev(str);
    
    return 0;
}

int atoi(const char* str) {
    int num = 0;
    uint8_t negative = 0;

    while(*str == ' ' || *str == '\n' || *str == '\r') str++;

    if(*str == '-') {
        negative = 1;
        str++;
    } else if(*str == '+') {
        str++;
    }

    while('0' <= *str && *str <= '9') {
        num *= 10;
        num += *str - '0';
        str++;
    }
    
    return negative ? num * -1 : num;
}
