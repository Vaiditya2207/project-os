/* Basic Memory Management */

#include "../kernel.h"

#define HEAP_START 0x100000  // 1MB
#define HEAP_SIZE 0x100000   // 1MB heap

static uint8_t* heap_start = (uint8_t*)HEAP_START;
static uint8_t* heap_current = (uint8_t*)HEAP_START;
static uint8_t* heap_end = (uint8_t*)(HEAP_START + HEAP_SIZE);

void memory_init(void) {
    // Initialize heap - but be very careful
    heap_current = heap_start;
    
    // Don't clear heap memory - this might be causing issues
    // memset(heap_start, 0, HEAP_SIZE);
    
    // Just set up basic pointers
}

// Simple bump allocator (for now)
void* kmalloc(size_t size) {
    // Align to 4-byte boundary
    size = (size + 3) & ~3;
    
    if (heap_current + size > heap_end) {
        return NULL;  // Out of memory
    }
    
    void* ptr = heap_current;
    heap_current += size;
    return ptr;
}

// Simple free (doesn't actually free memory yet)
void kfree(void* ptr) {
    // TODO: Implement proper free list
    (void)ptr;  // Suppress unused parameter warning
}
