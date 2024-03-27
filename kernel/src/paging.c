#include "../include/paging.h"

typedef struct {
    size_t vaddr;
    size_t paddr;
    uint8_t valid;
} tlb_entry_t;

static tlb_entry_t hw_tlb[TLB_MAX_ENTRIES];

void tlb_init() {
    for(size_t i = 0; i < TLB_MAX_ENTRIES; i++) {
        hw_tlb[i].valid = 0;
    }
}

int tlb_lookup(size_t vaddr, size_t* paddr) {
    for(size_t i = 0; i < TLB_MAX_ENTRIES; i++) {
        // if(hw_tlb[i] = 
    }
    return 0;
}
