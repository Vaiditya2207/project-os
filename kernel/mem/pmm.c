#include "pmm.h"
#include "../kernel.h"

// Global PMM state
static pmm_info_t pmm_info;
static uint8_t frame_bitmap[4096]; // 4KB bitmap = 32MB of RAM trackable (conservative)

/**
 * Initialize the Physical Memory Manager
 */
void pmm_init(void)
{
    vga_print("  Initializing Physical Memory Manager...\n");
    
    // Initialize PMM info structure
    pmm_info.total_frames = 0;
    pmm_info.used_frames = 0;
    pmm_info.free_frames = 0;
    pmm_info.reserved_frames = 0;
    pmm_info.frame_bitmap = frame_bitmap;
    pmm_info.bitmap_size = sizeof(frame_bitmap);
    
    // For now, assume 16MB of RAM starting at 1MB
    // This is a conservative estimate that works on most systems
    pmm_info.memory_start = 0x100000;  // 1MB
    pmm_info.memory_end = 0x1000000;   // 16MB
    pmm_info.memory_size = pmm_info.memory_end - pmm_info.memory_start;
    
    // Kernel occupies first 1MB of manageable memory
    pmm_info.kernel_start = 0x100000;  // 1MB
    pmm_info.kernel_end = 0x200000;    // 2MB
    
    // Clear the entire bitmap (mark all frames as used initially)
    for (uint32_t i = 0; i < pmm_info.bitmap_size; i++) {
        frame_bitmap[i] = 0xFF; // All frames marked as used
    }
    
    // Calculate total frames
    pmm_info.total_frames = pmm_info.memory_size / PAGE_SIZE;
    pmm_info.used_frames = pmm_info.total_frames; // All used initially
    pmm_info.free_frames = 0;
    
    // Initialize the available memory region (2MB - 16MB)
    uint32_t available_start = pmm_info.kernel_end;
    uint32_t available_size = pmm_info.memory_end - available_start;
    pmm_init_region(available_start, available_size);
    
    vga_print("    Memory range: 0x");
    vga_print_hex(pmm_info.memory_start);
    vga_print(" - 0x");
    vga_print_hex(pmm_info.memory_end);
    vga_print("\n");
    
    vga_print("    Total frames: ");
    vga_print_decimal(pmm_info.total_frames);
    vga_print(" (");
    vga_print_decimal(pmm_info.memory_size / 1024);
    vga_print("KB)\n");
    
    vga_print("    Available frames: ");
    vga_print_decimal(pmm_info.free_frames);
    vga_print("\n");
    
    vga_print("  Physical Memory Manager initialized\n");
}

/**
 * Initialize a region of memory as available
 */
void pmm_init_region(uint32_t start_addr, uint32_t size)
{
    // Align to page boundaries
    uint32_t aligned_start = PAGE_ALIGN_DOWN(start_addr);
    uint32_t aligned_end = PAGE_ALIGN_DOWN(start_addr + size);
    
    // Mark frames as available
    for (uint32_t addr = aligned_start; addr < aligned_end; addr += PAGE_SIZE) {
        pmm_clear_frame(addr);
    }
}

/**
 * Mark a region of memory as unavailable
 */
void pmm_deinit_region(uint32_t start_addr, uint32_t size)
{
    // Align to page boundaries
    uint32_t aligned_start = PAGE_ALIGN_DOWN(start_addr);
    uint32_t aligned_end = PAGE_ALIGN(start_addr + size);
    
    // Mark frames as used
    for (uint32_t addr = aligned_start; addr < aligned_end; addr += PAGE_SIZE) {
        pmm_set_frame(addr);
    }
}

/**
 * Allocate a single physical frame
 */
uint32_t pmm_alloc_frame(void)
{
    uint32_t frame = pmm_first_free_frame();
    
    if (frame == (uint32_t)-1) {
        // Out of memory
        return 0;
    }
    
    pmm_set_frame(frame);
    return frame;
}

/**
 * Free a single physical frame
 */
void pmm_free_frame(uint32_t frame_addr)
{
    // Align to frame boundary
    frame_addr = PAGE_ALIGN_DOWN(frame_addr);
    
    // Validate frame address
    if (frame_addr < pmm_info.memory_start || frame_addr >= pmm_info.memory_end) {
        return; // Invalid address
    }
    
    pmm_clear_frame(frame_addr);
}

/**
 * Allocate multiple contiguous frames
 */
uint32_t pmm_alloc_frames(uint32_t count)
{
    if (count == 0) return 0;
    if (count == 1) return pmm_alloc_frame();
    
    uint32_t start_frame = pmm_first_free_frames(count);
    
    if (start_frame == (uint32_t)-1) {
        return 0; // Not enough contiguous memory
    }
    
    // Mark all frames as used
    for (uint32_t i = 0; i < count; i++) {
        pmm_set_frame(start_frame + (i * PAGE_SIZE));
    }
    
    return start_frame;
}

/**
 * Free multiple contiguous frames
 */
void pmm_free_frames(uint32_t start_addr, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        pmm_free_frame(start_addr + (i * PAGE_SIZE));
    }
}

/**
 * Mark a frame as used
 */
void pmm_set_frame(uint32_t frame_addr)
{
    uint32_t frame = ADDR_TO_FRAME(frame_addr - pmm_info.memory_start);
    
    if (frame >= pmm_info.total_frames) return;
    
    uint32_t byte = frame / 8;
    uint8_t bit = frame % 8;
    
    // If frame was free, mark it as used
    if (!(frame_bitmap[byte] & (1 << bit))) {
        frame_bitmap[byte] |= (1 << bit);
        pmm_info.used_frames++;
        pmm_info.free_frames--;
    }
}

/**
 * Mark a frame as free
 */
void pmm_clear_frame(uint32_t frame_addr)
{
    uint32_t frame = ADDR_TO_FRAME(frame_addr - pmm_info.memory_start);
    
    if (frame >= pmm_info.total_frames) return;
    
    uint32_t byte = frame / 8;
    uint8_t bit = frame % 8;
    
    // If frame was used, mark it as free
    if (frame_bitmap[byte] & (1 << bit)) {
        frame_bitmap[byte] &= ~(1 << bit);
        pmm_info.used_frames--;
        pmm_info.free_frames++;
    }
}

/**
 * Test if a frame is allocated
 */
uint8_t pmm_test_frame(uint32_t frame_addr)
{
    uint32_t frame = ADDR_TO_FRAME(frame_addr - pmm_info.memory_start);
    
    if (frame >= pmm_info.total_frames) return 1; // Invalid = used
    
    uint32_t byte = frame / 8;
    uint8_t bit = frame % 8;
    
    return (frame_bitmap[byte] & (1 << bit)) ? 1 : 0;
}

/**
 * Find first free frame
 */
uint32_t pmm_first_free_frame(void)
{
    for (uint32_t byte = 0; byte < pmm_info.bitmap_size; byte++) {
        if (frame_bitmap[byte] != 0xFF) { // Found a byte with free frames
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (!(frame_bitmap[byte] & (1 << bit))) {
                    uint32_t frame = (byte * 8) + bit;
                    if (frame < pmm_info.total_frames) {
                        return pmm_info.memory_start + (frame * PAGE_SIZE);
                    }
                }
            }
        }
    }
    
    return (uint32_t)-1; // No free frames
}

/**
 * Find first set of contiguous free frames
 */
uint32_t pmm_first_free_frames(uint32_t count)
{
    uint32_t consecutive = 0;
    uint32_t start_frame = 0;
    
    for (uint32_t frame = 0; frame < pmm_info.total_frames; frame++) {
        uint32_t addr = pmm_info.memory_start + (frame * PAGE_SIZE);
        
        if (!pmm_test_frame(addr)) {
            // Frame is free
            if (consecutive == 0) {
                start_frame = frame;
            }
            consecutive++;
            
            if (consecutive >= count) {
                return pmm_info.memory_start + (start_frame * PAGE_SIZE);
            }
        } else {
            // Frame is used, reset counter
            consecutive = 0;
        }
    }
    
    return (uint32_t)-1; // Not enough contiguous frames
}

/**
 * Get PMM information structure
 */
pmm_info_t* pmm_get_info(void)
{
    return &pmm_info;
}

/**
 * Print memory statistics
 */
void pmm_print_stats(void)
{
    vga_print("Physical Memory Statistics:\n");
    vga_print("  Total memory: ");
    vga_print_decimal(pmm_info.memory_size / 1024);
    vga_print("KB (");
    vga_print_decimal(pmm_info.total_frames);
    vga_print(" frames)\n");
    
    vga_print("  Used memory: ");
    vga_print_decimal((pmm_info.used_frames * PAGE_SIZE) / 1024);
    vga_print("KB (");
    vga_print_decimal(pmm_info.used_frames);
    vga_print(" frames)\n");
    
    vga_print("  Free memory: ");
    vga_print_decimal((pmm_info.free_frames * PAGE_SIZE) / 1024);
    vga_print("KB (");
    vga_print_decimal(pmm_info.free_frames);
    vga_print(" frames)\n");
}

/**
 * Get total memory size
 */
uint32_t pmm_get_total_memory(void)
{
    return pmm_info.memory_size;
}

/**
 * Get free memory size
 */
uint32_t pmm_get_free_memory(void)
{
    return pmm_info.free_frames * PAGE_SIZE;
}

/**
 * Get used memory size
 */
uint32_t pmm_get_used_memory(void)
{
    return pmm_info.used_frames * PAGE_SIZE;
}

/**
 * Simple allocation test
 */
int pmm_test_allocation(void)
{
    vga_print("Testing PMM allocation...\n");
    
    // Test single frame allocation
    uint32_t frame1 = pmm_alloc_frame();
    if (frame1 == 0) {
        vga_print("  ERROR: Failed to allocate single frame\n");
        return 0;
    }
    vga_print("  Allocated frame at 0x");
    vga_print_hex(frame1);
    vga_print("\n");
    
    // Test multiple frame allocation
    uint32_t frames = pmm_alloc_frames(4);
    if (frames == 0) {
        vga_print("  ERROR: Failed to allocate 4 frames\n");
        return 0;
    }
    vga_print("  Allocated 4 frames starting at 0x");
    vga_print_hex(frames);
    vga_print("\n");
    
    // Free the allocations
    pmm_free_frame(frame1);
    pmm_free_frames(frames, 4);
    
    vga_print("  PMM allocation test passed\n");
    return 1;
}
