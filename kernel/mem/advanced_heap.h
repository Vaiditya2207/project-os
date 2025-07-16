/*
 * Advanced Heap Manager - SimpleOS v1.2.5
 * 
 * Features:
 * - Free list management with linked lists
 * - Block coalescing for fragmentation reduction
 * - Size classes for different allocation patterns
 * - Multiple allocation strategies (aligned, DMA, etc.)
 * - Best-fit and first-fit algorithms
 */

#ifndef ADVANCED_HEAP_H
#define ADVANCED_HEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Heap configuration
#define HEAP_MAGIC          0xDEADBEEF
#define HEAP_FREE_MAGIC     0xFEEDFACE
#define HEAP_MIN_BLOCK_SIZE 16
#define HEAP_ALIGNMENT      8

// Size classes for optimized allocation
#define SMALL_BLOCK_SIZE    64     // 0-64 bytes
#define MEDIUM_BLOCK_SIZE   1024   // 65-1024 bytes
#define LARGE_BLOCK_SIZE    4096   // 1025-4096 bytes
// Anything larger uses vmm_alloc_pages

// Allocation strategies
typedef enum {
    ALLOC_FIRST_FIT,    // Find first suitable block
    ALLOC_BEST_FIT,     // Find smallest suitable block
    ALLOC_WORST_FIT,    // Find largest suitable block (for debugging)
    ALLOC_NEXT_FIT      // Continue from last allocation point
} alloc_strategy_t;

// Block header structure
typedef struct block_header {
    uint32_t magic;                 // Magic number for corruption detection
    size_t size;                    // Size of the block (excluding header)
    bool is_free;                   // Whether block is free
    struct block_header *next;      // Next block in free list
    struct block_header *prev;      // Previous block in free list
    struct block_header *next_all;  // Next block in all blocks list
    struct block_header *prev_all;  // Previous block in all blocks list
} __attribute__((packed)) block_header_t;

// Size class structure
typedef struct {
    block_header_t *free_list;      // Head of free list for this size class
    uint32_t block_count;           // Number of blocks in this class
    uint32_t free_count;            // Number of free blocks
    uint32_t total_allocated;       // Total bytes allocated in this class
} size_class_t;

// Heap statistics
typedef struct {
    uint32_t total_size;            // Total heap size
    uint32_t used_size;             // Currently used bytes
    uint32_t free_size;             // Currently free bytes
    uint32_t largest_free_block;    // Size of largest free block
    uint32_t allocation_count;      // Total allocations made
    uint32_t free_count;            // Total frees made
    uint32_t coalesce_count;        // Number of block merges
    uint32_t fragmentation_percent; // Fragmentation percentage
} heap_stats_t;

// Main heap manager structure
typedef struct {
    uint8_t *heap_start;            // Start of heap memory
    uint8_t *heap_end;              // End of heap memory
    size_t heap_size;               // Total heap size
    block_header_t *all_blocks;     // Head of all blocks list
    size_class_t small_class;       // Small block size class
    size_class_t medium_class;      // Medium block size class
    size_class_t large_class;       // Large block size class
    alloc_strategy_t strategy;      // Current allocation strategy
    heap_stats_t stats;             // Heap statistics
    block_header_t *last_alloc;     // Last allocation point (for next-fit)
} heap_manager_t;

// Function declarations

// Core heap functions
void advanced_heap_init(void);
void* advanced_kmalloc(size_t size);
void advanced_kfree(void *ptr);
void* advanced_krealloc(void *ptr, size_t new_size);

// Specialized allocation functions
void* kmalloc_aligned(size_t size, size_t alignment);
void* kmalloc_dma(size_t size);
void* kcalloc(size_t num, size_t size);
void* kmalloc_atomic(size_t size);  // Non-blocking allocation

// Strategy and configuration
void heap_set_strategy(alloc_strategy_t strategy);
alloc_strategy_t heap_get_strategy(void);

// Statistics and debugging
void heap_print_stats(void);
void heap_dump_free_lists(void);
void heap_validate(void);
bool heap_check_corruption(void);
uint32_t heap_get_fragmentation(void);

// Internal functions
static block_header_t* find_free_block(size_t size, alloc_strategy_t strategy);
static void split_block(block_header_t *block, size_t size);
static void coalesce_blocks(block_header_t *block);
static void add_to_free_list(block_header_t *block);
static void remove_from_free_list(block_header_t *block);
static size_class_t* get_size_class(size_t size);
static size_t align_size(size_t size, size_t alignment);

// Utility functions
static inline bool is_aligned(void *ptr, size_t alignment) {
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

static inline size_t round_up_to_alignment(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

static inline bool is_valid_block(block_header_t *block) {
    return block && (block->magic == HEAP_MAGIC || 
                    (block->is_free && block->magic == HEAP_FREE_MAGIC));
}

// External heap manager instance
extern heap_manager_t heap_manager;

#endif // ADVANCED_HEAP_H
