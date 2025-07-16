#ifndef PMM_H
#define PMM_H

#include "../kernel.h"

// Physical Memory Manager constants
#define PAGE_SIZE 4096
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))
#define ADDR_TO_FRAME(addr) ((addr) / PAGE_SIZE)
#define FRAME_TO_ADDR(frame) ((frame) * PAGE_SIZE)

// Maximum supported physical memory (4GB)
#define MAX_MEMORY_SIZE (4UL * 1024 * 1024 * 1024)
#define MAX_FRAMES (MAX_MEMORY_SIZE / PAGE_SIZE)

// Memory map entry types (from bootloader/BIOS)
#define MEMORY_TYPE_AVAILABLE    1
#define MEMORY_TYPE_RESERVED     2
#define MEMORY_TYPE_ACPI_RECLAIM 3
#define MEMORY_TYPE_ACPI_NVS     4
#define MEMORY_TYPE_BAD          5

// Memory regions to avoid
#define KERNEL_START_ADDR    0x100000  // 1MB - where kernel is loaded
#define KERNEL_END_ADDR      0x200000  // 2MB - estimated kernel size
#define VGA_MEMORY_START     0xA0000   // VGA memory region
#define VGA_MEMORY_END       0x100000  // End of VGA region

// Physical memory manager information structure
typedef struct {
    uint32_t total_frames;      // Total number of frames
    uint32_t used_frames;       // Currently allocated frames
    uint32_t free_frames;       // Available frames
    uint32_t reserved_frames;   // Reserved/unusable frames
    
    uint8_t *frame_bitmap;      // Bitmap to track frame allocation
    uint32_t bitmap_size;       // Size of bitmap in bytes
    
    uint32_t memory_start;      // Start of manageable memory
    uint32_t memory_end;        // End of manageable memory
    uint32_t memory_size;       // Total manageable memory size
    
    uint32_t kernel_start;      // Kernel memory start
    uint32_t kernel_end;        // Kernel memory end
} pmm_info_t;

// Memory map entry structure (simplified)
typedef struct {
    uint32_t base_addr_low;     // Base address (low 32 bits)
    uint32_t base_addr_high;    // Base address (high 32 bits)
    uint32_t length_low;        // Length (low 32 bits)
    uint32_t length_high;       // Length (high 32 bits)
    uint32_t type;              // Memory type
} memory_map_entry_t;

// Function declarations
void pmm_init(void);
void pmm_init_region(uint32_t start_addr, uint32_t size);
void pmm_deinit_region(uint32_t start_addr, uint32_t size);

uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t frame_addr);
uint32_t pmm_alloc_frames(uint32_t count);
void pmm_free_frames(uint32_t start_addr, uint32_t count);

// Frame bitmap manipulation
void pmm_set_frame(uint32_t frame_addr);
void pmm_clear_frame(uint32_t frame_addr);
uint8_t pmm_test_frame(uint32_t frame_addr);
uint32_t pmm_first_free_frame(void);
uint32_t pmm_first_free_frames(uint32_t count);

// Memory information and statistics
pmm_info_t* pmm_get_info(void);
void pmm_print_stats(void);
uint32_t pmm_get_total_memory(void);
uint32_t pmm_get_free_memory(void);
uint32_t pmm_get_used_memory(void);

// Memory map parsing (for future bootloader integration)
void pmm_parse_memory_map(memory_map_entry_t *map, uint32_t entry_count);

// Debug and testing functions
void pmm_dump_bitmap(uint32_t start_frame, uint32_t count);
int pmm_test_allocation(void);

#endif
