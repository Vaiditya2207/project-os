/*
 * Virtual Memory Manager (VMM) Implementation - SimpleOS v1.2.5
 * 
 * Provides virtual memory management with paging support.
 */

#include "vmm.h"
#include "pmm.h"
#include "../kernel.h"

// Global VMM state
vmm_info_t vmm_info;

// Temporary page table for bootstrapping
static page_table_t bootstrap_page_table __attribute__((aligned(4096)));

/**
 * Initialize the Virtual Memory Manager
 */
void vmm_init(void) {
    vga_print("  Initializing Virtual Memory Manager...\n");
    
    // Initialize VMM info structure
    vmm_info.kernel_directory = NULL;
    vmm_info.current_directory = NULL;
    vmm_info.kernel_heap_current = KERNEL_HEAP_START;
    vmm_info.pages_allocated = 0;
    vmm_info.page_tables_allocated = 0;
    
    // Create kernel page directory
    vmm_info.kernel_directory = vmm_create_directory();
    if (!vmm_info.kernel_directory) {
        vga_print("    ERROR: Failed to create kernel page directory\n");
        return;
    }
    
    // Identity map the first 4MB (kernel space)
    // This ensures kernel code continues to work after paging is enabled
    for (uint32_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        if (!vmm_map_page(vmm_info.kernel_directory, addr, addr, 
                         PAGE_PRESENT | PAGE_WRITABLE)) {
            vga_print("    ERROR: Failed to identity map kernel space\n");
            return;
        }
    }
    
    // Map kernel to higher half (3GB+)
    // Virtual 0xC0000000+ maps to Physical 0x00000000+
    for (uint32_t addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        uint32_t virtual_addr = KERNEL_VIRTUAL_BASE + addr;
        if (!vmm_map_page(vmm_info.kernel_directory, virtual_addr, addr,
                         PAGE_PRESENT | PAGE_WRITABLE)) {
            vga_print("    ERROR: Failed to map kernel to higher half\n");
            return;
        }
    }
    
    // Switch to kernel page directory
    vmm_switch_directory(vmm_info.kernel_directory);
    
    // Enable paging
    uint32_t dir_physical = vmm_get_physical_addr(NULL, (uint32_t)vmm_info.kernel_directory);
    if (dir_physical == 0) {
        // If we can't translate yet, use identity mapping
        dir_physical = (uint32_t)vmm_info.kernel_directory;
    }
    
    vga_print("    Enabling paging with directory at 0x");
    vga_print_hex(dir_physical);
    vga_print("\n");
    
    vmm_enable_paging(dir_physical);
    
    vga_print("    Virtual Memory Manager initialized\n");
    vga_print("    Kernel virtual base: 0x");
    vga_print_hex(KERNEL_VIRTUAL_BASE);
    vga_print("\n");
}

/**
 * Create a new page directory
 */
page_directory_t* vmm_create_directory(void) {
    // Allocate a frame for the page directory
    uint32_t physical_addr = pmm_alloc_frame();
    if (physical_addr == 0) {
        return NULL;
    }
    
    // For now, use identity mapping to access the directory
    page_directory_t *dir = (page_directory_t*)physical_addr;
    
    // Clear the directory
    for (int i = 0; i < 1024; i++) {
        dir->entries[i] = 0;
    }
    
    vmm_info.page_tables_allocated++;
    
    return dir;
}

/**
 * Switch to a different page directory
 */
void vmm_switch_directory(page_directory_t *dir) {
    vmm_info.current_directory = dir;
    
    // Get physical address of directory
    uint32_t physical_addr = (uint32_t)dir; // Identity mapped for now
    
    // Load into CR3
    asm volatile("mov %0, %%cr3" : : "r" (physical_addr));
}

/**
 * Map a virtual page to a physical frame
 */
bool vmm_map_page(page_directory_t *dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    // Align addresses to page boundaries
    virtual_addr = PAGE_ALIGN_DOWN(virtual_addr);
    physical_addr = PAGE_ALIGN_DOWN(physical_addr);
    
    // Get page directory index
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virtual_addr);
    
    // Check if page table exists
    if (!(dir->entries[pd_index] & PAGE_PRESENT)) {
        // Allocate a new page table
        uint32_t table_physical = pmm_alloc_frame();
        if (table_physical == 0) {
            return false;
        }
        
        // Clear the page table
        page_table_t *table = (page_table_t*)table_physical;
        for (int i = 0; i < 1024; i++) {
            table->entries[i] = 0;
        }
        
        // Add page table to directory
        dir->entries[pd_index] = table_physical | PAGE_PRESENT | PAGE_WRITABLE | (flags & PAGE_USER);
        vmm_info.page_tables_allocated++;
    }
    
    // Get page table
    page_table_t *table = (page_table_t*)(dir->entries[pd_index] & ~0xFFF);
    
    // Map the page
    table->entries[pt_index] = physical_addr | flags;
    
    // Flush TLB entry
    vmm_flush_tlb_entry(virtual_addr);
    
    vmm_info.pages_allocated++;
    return true;
}

/**
 * Unmap a virtual page
 */
bool vmm_unmap_page(page_directory_t *dir, uint32_t virtual_addr) {
    virtual_addr = PAGE_ALIGN_DOWN(virtual_addr);
    
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virtual_addr);
    
    // Check if page table exists
    if (!(dir->entries[pd_index] & PAGE_PRESENT)) {
        return false; // Page not mapped
    }
    
    // Get page table
    page_table_t *table = (page_table_t*)(dir->entries[pd_index] & ~0xFFF);
    
    // Check if page is mapped
    if (!(table->entries[pt_index] & PAGE_PRESENT)) {
        return false; // Page not mapped
    }
    
    // Get physical address to free
    uint32_t physical_addr = table->entries[pt_index] & ~0xFFF;
    
    // Unmap the page
    table->entries[pt_index] = 0;
    
    // Free the physical frame
    pmm_free_frame(physical_addr);
    
    // Flush TLB entry
    vmm_flush_tlb_entry(virtual_addr);
    
    vmm_info.pages_allocated--;
    return true;
}

/**
 * Get physical address for a virtual address
 */
uint32_t vmm_get_physical_addr(page_directory_t *dir, uint32_t virtual_addr) {
    if (!dir) {
        dir = vmm_info.current_directory;
    }
    
    if (!dir) {
        return 0; // No page directory active
    }
    
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virtual_addr);
    uint32_t offset = virtual_addr & 0xFFF;
    
    // Check if page table exists
    if (!(dir->entries[pd_index] & PAGE_PRESENT)) {
        return 0; // Page table not present
    }
    
    // Get page table
    page_table_t *table = (page_table_t*)(dir->entries[pd_index] & ~0xFFF);
    
    // Check if page is mapped
    if (!(table->entries[pt_index] & PAGE_PRESENT)) {
        return 0; // Page not present
    }
    
    // Return physical address
    return (table->entries[pt_index] & ~0xFFF) | offset;
}

/**
 * Flush TLB entry for a specific address
 */
void vmm_flush_tlb_entry(uint32_t virtual_addr) {
    asm volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}

/**
 * Flush entire TLB
 */
void vmm_flush_tlb_all(void) {
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r" (cr3));
    asm volatile("mov %0, %%cr3" : : "r" (cr3));
}

/**
 * Allocate consecutive virtual pages in kernel space
 */
void* vmm_alloc_pages(uint32_t count) {
    uint32_t virtual_start = vmm_info.kernel_heap_current;
    
    // Check if we have space
    if (virtual_start + (count * PAGE_SIZE) >= KERNEL_HEAP_END) {
        return NULL; // Out of virtual address space
    }
    
    // Allocate physical frames and map them
    for (uint32_t i = 0; i < count; i++) {
        uint32_t physical_addr = pmm_alloc_frame();
        if (physical_addr == 0) {
            // Allocation failed, clean up
            for (uint32_t j = 0; j < i; j++) {
                vmm_unmap_page(vmm_info.kernel_directory, virtual_start + (j * PAGE_SIZE));
            }
            return NULL;
        }
        
        uint32_t virtual_addr = virtual_start + (i * PAGE_SIZE);
        if (!vmm_map_page(vmm_info.kernel_directory, virtual_addr, physical_addr,
                         PAGE_PRESENT | PAGE_WRITABLE)) {
            // Mapping failed, clean up
            pmm_free_frame(physical_addr);
            for (uint32_t j = 0; j < i; j++) {
                vmm_unmap_page(vmm_info.kernel_directory, virtual_start + (j * PAGE_SIZE));
            }
            return NULL;
        }
    }
    
    // Update heap pointer
    vmm_info.kernel_heap_current += count * PAGE_SIZE;
    
    return (void*)virtual_start;
}

/**
 * Free consecutive virtual pages
 */
void vmm_free_pages(void *virtual_addr, uint32_t count) {
    uint32_t addr = (uint32_t)virtual_addr;
    addr = PAGE_ALIGN_DOWN(addr);
    
    for (uint32_t i = 0; i < count; i++) {
        vmm_unmap_page(vmm_info.kernel_directory, addr + (i * PAGE_SIZE));
    }
}

/**
 * Print VMM statistics
 */
void vmm_print_stats(void) {
    vga_print("Virtual Memory Statistics:\n");
    vga_print("  Pages allocated: ");
    vga_print_decimal(vmm_info.pages_allocated);
    vga_print("\n");
    vga_print("  Page tables allocated: ");
    vga_print_decimal(vmm_info.page_tables_allocated);
    vga_print("\n");
    vga_print("  Kernel heap current: 0x");
    vga_print_hex(vmm_info.kernel_heap_current);
    vga_print("\n");
    vga_print("  Current page directory: 0x");
    vga_print_hex((uint32_t)vmm_info.current_directory);
    vga_print("\n");
}

/**
 * Test paging functionality
 */
void vmm_test_paging(void) {
    vga_print("Testing virtual memory allocation:\n");
    
    // Test page allocation
    void *pages = vmm_alloc_pages(2);
    if (pages) {
        vga_print("  Allocated 2 pages at: 0x");
        vga_print_hex((uint32_t)pages);
        vga_print("\n");
        
        // Test writing to the pages
        uint32_t *test_ptr = (uint32_t*)pages;
        *test_ptr = 0xDEADBEEF;
        
        if (*test_ptr == 0xDEADBEEF) {
            vga_print("  Memory write/read test: PASSED\n");
        } else {
            vga_print("  Memory write/read test: FAILED\n");
        }
        
        // Free the pages
        vmm_free_pages(pages, 2);
        vga_print("  Pages freed successfully\n");
    } else {
        vga_print("  Failed to allocate pages\n");
    }
}

/**
 * Get protection flags for a memory type
 */
uint32_t vmm_get_protection_flags(memory_type_t type) {
    switch (type) {
        case MEMORY_TYPE_KERNEL_CODE:
            return PAGE_KERNEL_RO | PAGE_GLOBAL;
        case MEMORY_TYPE_KERNEL_DATA:
        case MEMORY_TYPE_KERNEL_HEAP:
            return PAGE_KERNEL_RW | PAGE_GLOBAL;
        case MEMORY_TYPE_USER_CODE:
            return PAGE_USER_RO;
        case MEMORY_TYPE_USER_DATA:
        case MEMORY_TYPE_USER_STACK:
        case MEMORY_TYPE_USER_HEAP:
            return PAGE_USER_RW;
        default:
            return PAGE_KERNEL_RW;
    }
}

/**
 * Map a region of memory with specific protection
 */
bool vmm_map_region(page_directory_t *dir, uint32_t virtual_start, uint32_t size, memory_type_t type) {
    uint32_t flags = vmm_get_protection_flags(type);
    uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t virtual_addr = virtual_start + (i * PAGE_SIZE);
        uint32_t physical_addr = pmm_alloc_frame();
        
        if (physical_addr == 0) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                vmm_unmap_page(dir, virtual_start + (j * PAGE_SIZE));
            }
            return false;
        }
        
        if (!vmm_map_page(dir, virtual_addr, physical_addr, flags)) {
            pmm_free_frame(physical_addr);
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                vmm_unmap_page(dir, virtual_start + (j * PAGE_SIZE));
            }
            return false;
        }
    }
    
    return true;
}

/**
 * Change protection flags for a page
 */
bool vmm_protect_page(page_directory_t *dir, uint32_t virtual_addr, uint32_t new_flags) {
    virtual_addr = PAGE_ALIGN_DOWN(virtual_addr);
    
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virtual_addr);
    
    // Check if page table exists
    if (!(dir->entries[pd_index] & PAGE_PRESENT)) {
        return false;
    }
    
    // Get page table
    page_table_t *table = (page_table_t*)(dir->entries[pd_index] & ~0xFFF);
    
    // Check if page is mapped
    if (!(table->entries[pt_index] & PAGE_PRESENT)) {
        return false;
    }
    
    // Preserve physical address, update flags
    uint32_t physical_addr = table->entries[pt_index] & ~0xFFF;
    table->entries[pt_index] = physical_addr | new_flags;
    
    // Flush TLB entry
    vmm_flush_tlb_entry(virtual_addr);
    
    return true;
}

/**
 * Check if an address is valid for access
 */
bool vmm_is_address_valid(page_directory_t *dir, uint32_t virtual_addr, bool write_access, bool user_mode) {
    if (!dir) {
        dir = vmm_info.current_directory;
    }
    
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virtual_addr);
    
    // Check if page table exists
    if (!(dir->entries[pd_index] & PAGE_PRESENT)) {
        return false;
    }
    
    // Check page table permissions
    if (user_mode && !(dir->entries[pd_index] & PAGE_USER)) {
        return false; // User trying to access supervisor page table
    }
    
    // Get page table
    page_table_t *table = (page_table_t*)(dir->entries[pd_index] & ~0xFFF);
    
    // Check if page is present
    if (!(table->entries[pt_index] & PAGE_PRESENT)) {
        return false;
    }
    
    // Check user access
    if (user_mode && !(table->entries[pt_index] & PAGE_USER)) {
        return false; // User trying to access supervisor page
    }
    
    // Check write access
    if (write_access && !(table->entries[pt_index] & PAGE_WRITABLE)) {
        return false; // Trying to write to read-only page
    }
    
    return true;
}

/**
 * Create a new user process page directory
 */
page_directory_t* vmm_create_user_directory(void) {
    page_directory_t *user_dir = vmm_create_directory();
    if (!user_dir) {
        return NULL;
    }
    
    // Map kernel space (3GB-4GB) from kernel directory
    page_directory_t *kernel_dir = vmm_info.kernel_directory;
    for (uint32_t i = 768; i < 1024; i++) { // 768 = 3GB / 4MB
        user_dir->entries[i] = kernel_dir->entries[i];
    }
    
    return user_dir;
}

/**
 * Setup user stack
 */
bool vmm_setup_user_stack(page_directory_t *dir, uint32_t stack_top, uint32_t stack_size) {
    uint32_t stack_bottom = stack_top - stack_size;
    
    return vmm_map_region(dir, stack_bottom, stack_size, MEMORY_TYPE_USER_STACK);
}

/**
 * Setup user heap
 */
bool vmm_setup_user_heap(page_directory_t *dir, uint32_t heap_start, uint32_t heap_size) {
    return vmm_map_region(dir, heap_start, heap_size, MEMORY_TYPE_USER_HEAP);
}
