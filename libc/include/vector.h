#pragma once
#include "stdio.h"
#include "types.h"

// Define a structure for the generic vector
typedef struct {
    void *data;
    size_t element_size;
    size_t size;
    size_t capacity;
    void (*element_copy)(void *dest, const void *src);
    void (*element_free)(void *element);
} Vector;

// Function to initialize a generic vector
Vector* vector_init(size_t element_size, size_t initial_capacity,
                    void (*element_copy)(void *dest, const void *src),
                    void (*element_free)(void *element));

// Function to add an element to the vector
void vector_add(Vector* vector, const void* element);

// Function to get an element from the vector
bool vector_get(Vector* vector, size_t index, void* element);

// Function to set an element in the vector
bool vector_set(Vector* vector, size_t index, const void* element);

// Function to free the vector's memory
void vector_free(Vector* vector);

void no_free(void* element);

void unit8_copy(void* dest, const void* src);

void unit16_copy(void* dest, const void* src);

void unit32_copy(void* dest, const void* src);