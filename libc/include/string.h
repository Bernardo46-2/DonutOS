#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"

void* memcpy(void* src, void* dest, size_t size);
void* memset(void* ptr, int c, size_t n);
size_t strlen(const char* const str);
void strrev(char* str);
char* strcpy(char* dest, char* src);
char* strcat(char* dest, char* src);

#endif
