/* Basic Memory Management */

#include "../kernel.h"

#define HEAP_START 0x100000 // 1MB
#define HEAP_SIZE 0x100000  // 1MB heap

// Make these accessible for memory reporting
uint8_t *heap_start = (uint8_t *)HEAP_START;
uint8_t *heap_current = (uint8_t *)HEAP_START;
uint8_t *heap_end = (uint8_t *)(HEAP_START + HEAP_SIZE);

void memory_init(void)
{
    // Initialize heap - but be very careful
    heap_current = heap_start;

    // Don't clear heap memory - this might be causing issues
    // memset(heap_start, 0, HEAP_SIZE);

    // Just set up basic pointers
}

// Functional memory allocator with tracking
void *kmalloc(size_t size)
{
    // Align to 4-byte boundary
    size = (size + 3) & ~3;

    // Add header for tracking allocations
    size_t total_size = size + sizeof(size_t);

    if (heap_current + total_size > heap_end)
    {
        // Out of memory - return NULL
        return NULL;
    }

    void *ptr = heap_current;

    // Store size in header for potential future free() implementation
    *((size_t *)ptr) = size;

    // Move pointer past header
    void *user_ptr = (char *)ptr + sizeof(size_t);
    heap_current += total_size;

    // Clear the allocated memory to prevent garbage data issues
    for (size_t i = 0; i < size; i++)
    {
        ((char *)user_ptr)[i] = 0;
    }

    return user_ptr;
}

// Improved free function (still simple but safer)
void kfree(void *ptr)
{
    if (!ptr)
        return;

    // For now, just mark as freed but don't actually reclaim
    // In a full implementation, we'd maintain a free list
    // This prevents crashes while allowing memory tracking

    // Get back to the header
    void *header_ptr = (char *)ptr - sizeof(size_t);
    size_t size = *((size_t *)header_ptr);

    // Clear the memory to detect use-after-free bugs
    for (size_t i = 0; i < size; i++)
    {
        ((char *)ptr)[i] = 0xDE; // Dead beef pattern
    }

    // TODO: Add to free list for reuse
}
