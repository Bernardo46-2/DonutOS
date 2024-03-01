#include "../include/string.h"

void* memcpy(void* dest, void* src, size_t size) {
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

void strrev(void* str) {
    char* s = (char*)str;
    const size_t len = strlen(str);
    char tmp;

    for(size_t i = 0, j = len-1; i < j; i++, j--) {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

void* strcpy(void* dest, void* src) {
    char* d = (char*)dest;
    char* s = (char*)src;

    while((*d = *s) != '\0') {
        d++;
        s++;
    }
    
    return dest;
}

void* strcat(void* dest, void* src) {
    strcpy(dest + strlen(dest), src);
    return dest;
}

void* memmove(void* dest, void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    uint8_t* s = (uint8_t*)src;

    if(s == d || n == 0)
        return dest;
    else if(d > s && d - s < n)
        for(size_t i = n-1; i >= 0; i--)
            d[i] = s[i];
    else if(d < s && s - d < n)
        for(size_t i = 0; i < n; i++)
            d[i] = s[i];
    else
        memcpy(dest, src, n);
    
    return dest;
}

int strcmp(void* s1, void* s2) {
    char* a = (char*)s1;
    char* b = (char*)s2;
    
    while(*a == *b++)
        if(*a++ == '\0')
            return 0;
    return (*a - *(b-1));
}
