#include "../include/string.h"

void* memcpy(void* src, void* dest, size_t size) {
    uint8_t* s = (uint8_t*)src;
    uint8_t* d = (uint8_t*)dest;

    while(size--) 
        *d++ = *s++;
    
    return dest;
}

void* memset(void* ptr, int c, size_t n) {
    uint8_t* p = (uint8_t*)ptr;
    while(n--) *p++ = c;
    return ptr;
}

size_t strlen(const char* const str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}

void strrev(char* str) {
    const size_t len = strlen(str);
    char tmp;

    for(size_t i = 0, j = len-1; i < j; i++, j--) {
        tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
    }
}

char* strcpy(char* dest, char* src) {
    char* d = dest;

    while((*dest = *src) != '\0') {
        dest++;
        src++;
    }
    
    return d;
}

char* strcat(char* dest, char* src) {
    strcpy(dest + strlen(dest), src);
    return dest;
}
