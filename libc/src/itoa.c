#include "../include/itoa.h"
#include "../include/string.h"

int itoa(int value, char* str, int len, int base) {
    if (len == 0 || base < 2 || base > 36) return -1;

    unsigned int num = value; // Trabalha com num como unsigned
    size_t i = 0;

    do {
        unsigned int digit = num % base;
        str[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    } while ((num /= base) && (i < (len - 1)));

    if (i == (len - 1) && num) return -1; // Verifica se ocorreu estouro

    str[i] = '\0';

    strrev(str);

    return 0;
}

