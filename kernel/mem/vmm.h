/*
 * Virtual Memory Manager (VMM) - SimpleOS v1.2.5
 * 
 * Implements virtual memory management with paging support.
 * Features:
 * - Page directory and page table management
 * - Virtual to physical address translation
 * - Memory mapping and unmapping
 * - Page fault handling
 */

#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stdbool.h>

// Page size constants
#define PAGE_SIZE           4096
#define PAGE_ALIGN_DOWN(x)  ((x) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(x)    (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

// Virtual memory layout
#define KERNEL_VIRTUAL_BASE 0xC0000000  // 3GB - Higher half kernel
#define USER_SPACE_END      0xC0000000  // 0-3GB user space
#define KERNEL_HEAP_START   0xD0000000  // Kernel heap area
#define KERNEL_HEAP_END     0xE0000000  // 256MB kernel heap

// User space layout
#define USER_CODE_BASE      0x08048000  // Standard ELF load address
#define USER_HEAP_START     0x10000000  // User heap starts at 256MB
#define USER_HEAP_END       0x80000000  // User heap ends at 2GB
#define USER_STACK_TOP      0xBFFFF000  // User stack top (grows down)
#define USER_STACK_SIZE     0x00100000  // 1MB default stack size

// Page directory and table indices
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x)     (((x) >> 12) & 0x3FF)
#define PAGE_GET_PHYSICAL_ADDR(x) ((x) & ~0xFFF)

// Page flags
#define PAGE_PRESENT    0x001   // Page is present in memory
#define PAGE_WRITABLE   0x002   // Page is writable
#define PAGE_USER       0x004   // Page is accessible by user mode
#define PAGE_WRITETHROUGH 0x008 // Write-through caching
#define PAGE_CACHE_DISABLE 0x010 // Disable caching
#define PAGE_ACCESSED   0x020   // Page has been accessed
#define PAGE_DIRTY      0x040   // Page has been written to
#define PAGE_4MB        0x080   // 4MB page (for page directory entries)
#define PAGE_GLOBAL     0x100   // Global page (not flushed on CR3 reload)

// Protection flags combinations
#define PAGE_KERNEL_RO  (PAGE_PRESENT)                                    // Kernel read-only
#define PAGE_KERNEL_RW  (PAGE_PRESENT | PAGE_WRITABLE)                   // Kernel read-write
#define PAGE_USER_RO    (PAGE_PRESENT | PAGE_USER)                       // User read-only
#define PAGE_USER_RW    (PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER)       // User read-write

// Memory protection types
typedef enum {
    MEMORY_TYPE_KERNEL_CODE,    // Kernel code (read-only, supervisor)
    MEMORY_TYPE_KERNEL_DATA,    // Kernel data (read-write, supervisor)
    MEMORY_TYPE_KERNEL_HEAP,    // Kernel heap (read-write, supervisor)
    MEMORY_TYPE_USER_CODE,      // User code (read-only, user)
    MEMORY_TYPE_USER_DATA,      // User data (read-write, user)
    MEMORY_TYPE_USER_STACK,     // User stack (read-write, user)
    MEMORY_TYPE_USER_HEAP       // User heap (read-write, user)
} memory_type_t;

// Page table entry type
typedef uint32_t page_table_entry_t;
typedef uint32_t page_directory_entry_t;

// Page directory structure (1024 entries)
typedef struct {
    page_directory_entry_t entries[1024];
} __attribute__((packed)) page_directory_t;

// Page table structure (1024 entries)
typedef struct {
    page_table_entry_t entries[1024];
} __attribute__((packed)) page_table_t;

// Virtual memory manager info
typedef struct {
    page_directory_t *kernel_directory;    // Kernel page directory
    page_directory_t *current_directory;   // Currently active directory
    uint32_t kernel_heap_current;          // Current kernel heap position
    uint32_t pages_allocated;              // Total pages allocated
    uint32_t page_tables_allocated;        // Total page tables allocated
} vmm_info_t;

// VMM function declarations
void vmm_init(void);
void vmm_switch_directory(page_directory_t *dir);
page_directory_t* vmm_create_directory(void);
void vmm_destroy_directory(page_directory_t *dir);

// Page mapping functions
bool vmm_map_page(page_directory_t *dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
bool vmm_unmap_page(page_directory_t *dir, uint32_t virtual_addr);
uint32_t vmm_get_physical_addr(page_directory_t *dir, uint32_t virtual_addr);

// Memory protection functions
bool vmm_map_region(page_directory_t *dir, uint32_t virtual_start, uint32_t size, memory_type_t type);
bool vmm_protect_page(page_directory_t *dir, uint32_t virtual_addr, uint32_t new_flags);
bool vmm_is_address_valid(page_directory_t *dir, uint32_t virtual_addr, bool write_access, bool user_mode);
uint32_t vmm_get_protection_flags(memory_type_t type);

// User space management
page_directory_t* vmm_create_user_directory(void);
bool vmm_setup_user_stack(page_directory_t *dir, uint32_t stack_top, uint32_t stack_size);
bool vmm_setup_user_heap(page_directory_t *dir, uint32_t heap_start, uint32_t heap_size);

// Kernel heap functions
void* vmm_alloc_pages(uint32_t count);
void vmm_free_pages(void *virtual_addr, uint32_t count);

// Page table manipulation
page_table_t* vmm_get_page_table(page_directory_t *dir, uint32_t virtual_addr, bool create);
void vmm_flush_tlb_entry(uint32_t virtual_addr);
void vmm_flush_tlb_all(void);

// Statistics and debugging
void vmm_print_stats(void);
void vmm_dump_page_directory(page_directory_t *dir);
void vmm_test_paging(void);

// Helper functions
static inline void vmm_enable_paging(uint32_t page_directory_physical) {
    asm volatile(
        "mov %0, %%cr3\n\t"     // Load page directory
        "mov %%cr0, %%eax\n\t"  // Get CR0
        "or $0x80000000, %%eax\n\t" // Set paging bit
        "mov %%eax, %%cr0\n\t"  // Enable paging
        :
        : "r" (page_directory_physical)
        : "eax"
    );
}

static inline void vmm_disable_paging(void) {
    asm volatile(
        "mov %%cr0, %%eax\n\t"      // Get CR0
        "and $0x7FFFFFFF, %%eax\n\t" // Clear paging bit
        "mov %%eax, %%cr0\n\t"      // Disable paging
        :
        :
        : "eax"
    );
}

static inline uint32_t vmm_get_page_directory_physical(void) {
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r" (cr3));
    return cr3;
}

// External declarations
extern vmm_info_t vmm_info;

#endif // VMM_H
