#include "stdlib.h"
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
                    void (*element_free)(void *element)) {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    vector->data = malloc(initial_capacity * element_size);
    vector->element_size = element_size;
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->element_copy = element_copy;
    vector->element_free = element_free;
    return vector;
}

// Function to add an element to the vector
void vector_add(Vector* vector, const void* element) {
    if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        vector->data = realloc(vector->data, vector->capacity * vector->element_size);
    }
    void* dest = (char*)vector->data + (vector->size * vector->element_size);
    vector->element_copy(dest, element);
    vector->size++;
}

// Function to get an element from the vector
bool vector_get(Vector* vector, size_t index, void* element) {
    if (index >= vector->size) return false;
    
    void* src = (char*)vector->data + (index * vector->element_size);
    vector->element_copy(element, src);
    return true;
}

// Function to set an element in the vector
bool vector_set(Vector* vector, size_t index, const void* element) {
    if (index >= vector->size) return false;
    
    void* dest = (char*)vector->data + (index * vector->element_size);
    vector->element_free(dest);
    vector->element_copy(dest, element);
    return true;
}

// Function to free the vector's memory
void vector_free(Vector* vector) {
    for (size_t i = 0; i < vector->size; ++i) {
        void* element = (char*)vector->data + (i * vector->element_size);
        vector->element_free(element);
    }
    free(vector->data);
    free(vector);
}

void no_free(void* element) {
    // Do nothing
}


void unit8_copy(void* dest, const void* src) {
    *(uint8_t*)dest = *(const uint8_t*)src;
}

void unit16_copy(void* dest, const void* src) {
    *(uint16_t*)dest = *(const uint16_t*)src;
}

void unit32_copy(void* dest, const void* src) {
    *(uint32_t*)dest = *(const uint32_t*)src;
}

void size_t_copy(void* dest, const void* src) {
    *(size_t*)dest = *(const size_t*)src;
}