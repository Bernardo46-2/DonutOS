#include "../include/itoa.h"
#include "../include/string.h"

int itoa(int num, char* str, int len, int base) {
    if(len == 0) return -1;
    
    size_t i = 0;
    int digit;
    
    do {
        digit = num % base;

        if(digit < 0xa) str[i++] = '0' + digit;
        else str[i++] = 'a' + digit - 0xa;

        num /= base;
    } while(num && (i < (len-1)));

    if(i == (len-1) && num) return -1;

    str[i] = '\0';
    strrev(str);
    
    return 0;
}
