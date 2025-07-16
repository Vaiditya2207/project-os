/*
 * Advanced Heap Manager Implementation - SimpleOS v1.2.5
 */

#include "advanced_heap.h"
#include "vmm.h"
#include "../kernel.h"

// Global heap manager instance
heap_manager_t heap_manager;

/**
 * Initialize the advanced heap manager
 */
void advanced_heap_init(void) {
    vga_print("  Initializing Advanced Heap Manager...\n");
    
    // Use VMM to allocate initial heap pages
    heap_manager.heap_size = 1024 * 1024; // 1MB initial heap
    heap_manager.heap_start = (uint8_t*)vmm_alloc_pages(heap_manager.heap_size / PAGE_SIZE);
    
    if (!heap_manager.heap_start) {
        vga_print("    ERROR: Failed to allocate heap pages\n");
        return;
    }
    
    heap_manager.heap_end = heap_manager.heap_start + heap_manager.heap_size;
    heap_manager.all_blocks = NULL;
    heap_manager.strategy = ALLOC_FIRST_FIT;
    heap_manager.last_alloc = NULL;
    
    // Initialize size classes
    heap_manager.small_class.free_list = NULL;
    heap_manager.small_class.block_count = 0;
    heap_manager.small_class.free_count = 0;
    heap_manager.small_class.total_allocated = 0;
    
    heap_manager.medium_class.free_list = NULL;
    heap_manager.medium_class.block_count = 0;
    heap_manager.medium_class.free_count = 0;
    heap_manager.medium_class.total_allocated = 0;
    
    heap_manager.large_class.free_list = NULL;
    heap_manager.large_class.block_count = 0;
    heap_manager.large_class.free_count = 0;
    heap_manager.large_class.total_allocated = 0;
    
    // Initialize statistics
    heap_manager.stats.total_size = heap_manager.heap_size;
    heap_manager.stats.used_size = 0;
    heap_manager.stats.free_size = heap_manager.heap_size;
    heap_manager.stats.largest_free_block = heap_manager.heap_size;
    heap_manager.stats.allocation_count = 0;
    heap_manager.stats.free_count = 0;
    heap_manager.stats.coalesce_count = 0;
    heap_manager.stats.fragmentation_percent = 0;
    
    // Create initial free block covering entire heap
    block_header_t *initial_block = (block_header_t*)heap_manager.heap_start;
    initial_block->magic = HEAP_FREE_MAGIC;
    initial_block->size = heap_manager.heap_size - sizeof(block_header_t);
    initial_block->is_free = true;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    initial_block->next_all = NULL;
    initial_block->prev_all = NULL;
    
    // Add to appropriate free list
    heap_manager.all_blocks = initial_block;
    add_to_free_list(initial_block);
    
    vga_print("    Advanced Heap Manager initialized\n");
    vga_print("    Heap size: ");
    vga_print_decimal(heap_manager.heap_size / 1024);
    vga_print("KB at 0x");
    vga_print_hex((uint32_t)heap_manager.heap_start);
    vga_print("\n");
}

/**
 * Get size class for a given size
 */
static size_class_t* get_size_class(size_t size) {
    if (size <= SMALL_BLOCK_SIZE) {
        return &heap_manager.small_class;
    } else if (size <= MEDIUM_BLOCK_SIZE) {
        return &heap_manager.medium_class;
    } else {
        return &heap_manager.large_class;
    }
}

/**
 * Add block to appropriate free list
 */
static void add_to_free_list(block_header_t *block) {
    if (!block || !block->is_free) return;
    
    size_class_t *size_class = get_size_class(block->size);
    
    // Add to front of free list
    block->next = size_class->free_list;
    block->prev = NULL;
    
    if (size_class->free_list) {
        size_class->free_list->prev = block;
    }
    
    size_class->free_list = block;
    size_class->free_count++;
}

/**
 * Remove block from free list
 */
static void remove_from_free_list(block_header_t *block) {
    if (!block) return;
    
    size_class_t *size_class = get_size_class(block->size);
    
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        size_class->free_list = block->next;
    }
    
    if (block->next) {
        block->next->prev = block->prev;
    }
    
    block->next = NULL;
    block->prev = NULL;
    size_class->free_count--;
}

/**
 * Find free block using specified strategy
 */
static block_header_t* find_free_block(size_t size, alloc_strategy_t strategy) {
    block_header_t *best_block = NULL;
    block_header_t *current;
    
    // Try size classes first for better performance
    size_class_t *size_class = get_size_class(size);
    
    switch (strategy) {
        case ALLOC_FIRST_FIT:
            // Check appropriate size class first
            for (current = size_class->free_list; current; current = current->next) {
                if (current->is_free && current->size >= size) {
                    return current;
                }
            }
            
            // Check larger size classes if needed
            if (size <= SMALL_BLOCK_SIZE) {
                for (current = heap_manager.medium_class.free_list; current; current = current->next) {
                    if (current->is_free && current->size >= size) {
                        return current;
                    }
                }
                for (current = heap_manager.large_class.free_list; current; current = current->next) {
                    if (current->is_free && current->size >= size) {
                        return current;
                    }
                }
            } else if (size <= MEDIUM_BLOCK_SIZE) {
                for (current = heap_manager.large_class.free_list; current; current = current->next) {
                    if (current->is_free && current->size >= size) {
                        return current;
                    }
                }
            }
            break;
            
        case ALLOC_BEST_FIT:
            // Find smallest suitable block
            for (current = heap_manager.all_blocks; current; current = current->next_all) {
                if (current->is_free && current->size >= size) {
                    if (!best_block || current->size < best_block->size) {
                        best_block = current;
                    }
                }
            }
            break;
            
        case ALLOC_WORST_FIT:
            // Find largest suitable block
            for (current = heap_manager.all_blocks; current; current = current->next_all) {
                if (current->is_free && current->size >= size) {
                    if (!best_block || current->size > best_block->size) {
                        best_block = current;
                    }
                }
            }
            break;
            
        case ALLOC_NEXT_FIT:
            // Start from last allocation point
            current = heap_manager.last_alloc ? heap_manager.last_alloc->next_all : heap_manager.all_blocks;
            
            while (current) {
                if (current->is_free && current->size >= size) {
                    return current;
                }
                current = current->next_all;
            }
            
            // Wrap around to beginning
            for (current = heap_manager.all_blocks; 
                 current && current != heap_manager.last_alloc; 
                 current = current->next_all) {
                if (current->is_free && current->size >= size) {
                    return current;
                }
            }
            break;
    }
    
    return best_block;
}

/**
 * Split a block if it's larger than needed
 */
static void split_block(block_header_t *block, size_t size) {
    if (!block || block->size <= size + sizeof(block_header_t) + HEAP_MIN_BLOCK_SIZE) {
        return; // Block too small to split
    }
    
    // Create new block from remainder
    block_header_t *new_block = (block_header_t*)((uint8_t*)block + sizeof(block_header_t) + size);
    new_block->magic = HEAP_FREE_MAGIC;
    new_block->size = block->size - size - sizeof(block_header_t);
    new_block->is_free = true;
    new_block->next = NULL;
    new_block->prev = NULL;
    
    // Update linked list pointers
    new_block->next_all = block->next_all;
    new_block->prev_all = block;
    
    if (block->next_all) {
        block->next_all->prev_all = new_block;
    }
    block->next_all = new_block;
    
    // Update original block
    block->size = size;
    
    // Add new block to free list
    add_to_free_list(new_block);
}

/**
 * Coalesce adjacent free blocks
 */
static void coalesce_blocks(block_header_t *block) {
    if (!block || !block->is_free) return;
    
    // Coalesce with next block
    block_header_t *next_block = block->next_all;
    if (next_block && next_block->is_free &&
        (uint8_t*)block + sizeof(block_header_t) + block->size == (uint8_t*)next_block) {
        
        remove_from_free_list(next_block);
        
        block->size += sizeof(block_header_t) + next_block->size;
        block->next_all = next_block->next_all;
        
        if (next_block->next_all) {
            next_block->next_all->prev_all = block;
        }
        
        heap_manager.stats.coalesce_count++;
    }
    
    // Coalesce with previous block
    block_header_t *prev_block = block->prev_all;
    if (prev_block && prev_block->is_free &&
        (uint8_t*)prev_block + sizeof(block_header_t) + prev_block->size == (uint8_t*)block) {
        
        remove_from_free_list(block);
        remove_from_free_list(prev_block);
        
        prev_block->size += sizeof(block_header_t) + block->size;
        prev_block->next_all = block->next_all;
        
        if (block->next_all) {
            block->next_all->prev_all = prev_block;
        }
        
        add_to_free_list(prev_block);
        heap_manager.stats.coalesce_count++;
    }
}

/**
 * Advanced malloc implementation
 */
void* advanced_kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Align size to heap alignment
    size = round_up_to_alignment(size, HEAP_ALIGNMENT);
    
    // For very large allocations, use VMM directly
    if (size > LARGE_BLOCK_SIZE) {
        uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        return vmm_alloc_pages(pages);
    }
    
    // Find suitable free block
    block_header_t *block = find_free_block(size, heap_manager.strategy);
    if (!block) {
        return NULL; // Out of memory
    }
    
    // Remove from free list
    remove_from_free_list(block);
    
    // Split block if necessary
    split_block(block, size);
    
    // Mark as allocated
    block->magic = HEAP_MAGIC;
    block->is_free = false;
    
    // Update statistics
    heap_manager.stats.allocation_count++;
    heap_manager.stats.used_size += size;
    heap_manager.stats.free_size -= size;
    
    size_class_t *size_class = get_size_class(size);
    size_class->total_allocated += size;
    
    // Update last allocation for next-fit
    heap_manager.last_alloc = block;
    
    return (uint8_t*)block + sizeof(block_header_t);
}

/**
 * Advanced free implementation
 */
void advanced_kfree(void *ptr) {
    if (!ptr) return;
    
    // Get block header
    block_header_t *block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    
    // Validate block
    if (!is_valid_block(block) || block->is_free) {
        return; // Invalid or already freed
    }
    
    // Mark as free
    block->magic = HEAP_FREE_MAGIC;
    block->is_free = true;
    
    // Update statistics
    heap_manager.stats.free_count++;
    heap_manager.stats.used_size -= block->size;
    heap_manager.stats.free_size += block->size;
    
    size_class_t *size_class = get_size_class(block->size);
    size_class->total_allocated -= block->size;
    
    // Add to free list
    add_to_free_list(block);
    
    // Coalesce with adjacent blocks
    coalesce_blocks(block);
}

/**
 * Aligned allocation
 */
void* kmalloc_aligned(size_t size, size_t alignment) {
    if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return NULL; // Invalid parameters
    }
    
    // Allocate extra space for alignment
    size_t extra_size = size + alignment + sizeof(void*);
    void *raw_ptr = advanced_kmalloc(extra_size);
    if (!raw_ptr) return NULL;
    
    // Find aligned address
    uintptr_t aligned_addr = ((uintptr_t)raw_ptr + sizeof(void*) + alignment - 1) & ~(alignment - 1);
    void *aligned_ptr = (void*)aligned_addr;
    
    // Store original pointer before aligned pointer
    *((void**)aligned_ptr - 1) = raw_ptr;
    
    return aligned_ptr;
}

/**
 * Zero-initialized allocation
 */
void* kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void *ptr = advanced_kmalloc(total_size);
    
    if (ptr) {
        // Clear memory
        for (size_t i = 0; i < total_size; i++) {
            ((uint8_t*)ptr)[i] = 0;
        }
    }
    
    return ptr;
}

/**
 * DMA-coherent allocation (for now, same as regular allocation)
 */
void* kmalloc_dma(size_t size) {
    return advanced_kmalloc(size);
}

/**
 * Realloc implementation
 */
void* advanced_krealloc(void *ptr, size_t new_size) {
    if (!ptr) return advanced_kmalloc(new_size);
    if (new_size == 0) {
        advanced_kfree(ptr);
        return NULL;
    }
    
    block_header_t *block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    if (!is_valid_block(block) || block->is_free) {
        return NULL;
    }
    
    if (new_size <= block->size) {
        return ptr; // Current block is large enough
    }
    
    // Allocate new block and copy data
    void *new_ptr = advanced_kmalloc(new_size);
    if (!new_ptr) return NULL;
    
    // Copy old data
    for (size_t i = 0; i < block->size && i < new_size; i++) {
        ((uint8_t*)new_ptr)[i] = ((uint8_t*)ptr)[i];
    }
    
    advanced_kfree(ptr);
    return new_ptr;
}

/**
 * Print heap statistics
 */
void heap_print_stats(void) {
    vga_print("Advanced Heap Statistics:\n");
    vga_print("  Total size: ");
    vga_print_decimal(heap_manager.stats.total_size / 1024);
    vga_print("KB\n");
    vga_print("  Used: ");
    vga_print_decimal(heap_manager.stats.used_size / 1024);
    vga_print("KB, Free: ");
    vga_print_decimal(heap_manager.stats.free_size / 1024);
    vga_print("KB\n");
    vga_print("  Allocations: ");
    vga_print_decimal(heap_manager.stats.allocation_count);
    vga_print(", Frees: ");
    vga_print_decimal(heap_manager.stats.free_count);
    vga_print("\n");
    vga_print("  Coalesces: ");
    vga_print_decimal(heap_manager.stats.coalesce_count);
    vga_print("\n");
    vga_print("  Size classes - Small: ");
    vga_print_decimal(heap_manager.small_class.free_count);
    vga_print(" free, Medium: ");
    vga_print_decimal(heap_manager.medium_class.free_count);
    vga_print(" free, Large: ");
    vga_print_decimal(heap_manager.large_class.free_count);
    vga_print(" free\n");
}

/**
 * Test advanced heap functionality
 */
void heap_test_advanced(void) {
    vga_print("Testing Advanced Heap:\n");
    
    // Test different allocation sizes
    void *small_ptr = advanced_kmalloc(32);
    void *medium_ptr = advanced_kmalloc(512);
    void *large_ptr = advanced_kmalloc(2048);
    
    vga_print("  Allocated small (32B): ");
    vga_print(small_ptr ? "SUCCESS" : "FAILED");
    vga_print("\n");
    
    vga_print("  Allocated medium (512B): ");
    vga_print(medium_ptr ? "SUCCESS" : "FAILED");
    vga_print("\n");
    
    vga_print("  Allocated large (2KB): ");
    vga_print(large_ptr ? "SUCCESS" : "FAILED");
    vga_print("\n");
    
    // Test aligned allocation
    void *aligned_ptr = kmalloc_aligned(100, 64);
    vga_print("  Aligned allocation (64B): ");
    vga_print((aligned_ptr && is_aligned(aligned_ptr, 64)) ? "SUCCESS" : "FAILED");
    vga_print("\n");
    
    // Test calloc
    void *zero_ptr = kcalloc(10, 16);
    vga_print("  Zero-initialized allocation: ");
    vga_print(zero_ptr ? "SUCCESS" : "FAILED");
    vga_print("\n");
    
    // Free everything
    advanced_kfree(small_ptr);
    advanced_kfree(medium_ptr);
    advanced_kfree(large_ptr);
    advanced_kfree(aligned_ptr);
    advanced_kfree(zero_ptr);
    
    vga_print("  All pointers freed\n");
}
