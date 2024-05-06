#include "../include/string.h"

void* memcpy(void* dest, const void* src, size_t size) {
    size_t i;

    // Copy a word at a time
    for(i = 0; i < size / sizeof(size_t); i++) {
        ((size_t*)dest)[i] = ((size_t*)src)[i];
    }

    // Copy the remaining bytes
    for(i = i * sizeof(size_t); i < size; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
    
    return dest;
}

void* memset(void* ptr, uint8_t c, size_t n) {
    size_t c_arch = 0, i;

    // Create a word-sized value of c
    for (i = 0; i < sizeof(size_t); i++) {
        c_arch |= ((size_t)c) << (i * 8);
    }

    // Set memory in word-sized chunks
    for (i = 0; i < n / sizeof(size_t); i ++) {
        ((size_t*)ptr)[i] = c_arch;
    }

    // Set the remaining bytes
    for (i  *= sizeof(size_t); i < n; i++) {
        ((uint8_t*)ptr)[i] = c;
    }

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

    for(size_t i = 0; i < len / 2; i++) {
        tmp = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = tmp;
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
    else if(d > s && d - s < n) {
        // Move data word by word in reverse order
        size_t num_words = n / sizeof(size_t);
        size_t num_bytes = n % sizeof(size_t);
        for(size_t i = num_words; i > 0; i--) {
            ((size_t*)d)[i-1] = ((size_t*)s)[i-1];
        }
        // Move the remaining bytes
        for(size_t i = 0; i < num_bytes; i++) {
            d[num_words * sizeof(size_t) + i] = s[num_words * sizeof(size_t) + i];
        }
    }
    else if(d < s && s - d < n) {
        // Move data word by word in forward order
        size_t num_words = n / sizeof(size_t);
        size_t num_bytes = n % sizeof(size_t);
        for(size_t i = 0; i < num_words; i++) {
            ((size_t*)d)[i] = ((size_t*)s)[i];
        }
        // Move the remaining bytes
        for(size_t i = 0; i < num_bytes; i++) {
            d[num_words * sizeof(size_t) + i] = s[num_words * sizeof(size_t) + i];
        }
    }
    else {
        // Move data using memcpy
        memcpy(dest, src, n);
    }
    
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
