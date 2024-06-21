#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"

void* memcpy(void* dest, const void* src, size_t size);
void* memset(void* ptr, uint8_t c, size_t n);
size_t strlen(const char* const str);
void strrev(void* str);
void* strcpy(void* dest, void* src);
void* strcat(void* dest, void* src);
void* memmove(void* dest, void* src, size_t n);
int strcmp(void* s1, void* s2);

#endif
