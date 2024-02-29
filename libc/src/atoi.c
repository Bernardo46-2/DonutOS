#include "../include/atoi.h"

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
