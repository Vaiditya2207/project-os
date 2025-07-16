#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// VGA Driver
void vga_init(void);
void vga_clear(void);
void vga_print(const char *str);
void vga_print_hex(uint32_t value);
void vga_print_decimal(uint32_t value);
void vga_putchar(char c);
void vga_set_color(uint8_t foreground, uint8_t background);

// Memory Management
void memory_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

// Physical Memory Manager
void pmm_init(void);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t frame_addr);
uint32_t pmm_alloc_frames(uint32_t count);
void pmm_free_frames(uint32_t start_addr, uint32_t count);
void pmm_print_stats(void);
uint32_t pmm_get_free_memory(void);
int pmm_test_allocation(void);

// Virtual Memory Manager
void vmm_init(void);
void vmm_print_stats(void);
void vmm_test_paging(void);
void* vmm_alloc_pages(uint32_t count);
void vmm_free_pages(void *virtual_addr, uint32_t count);

// Advanced Heap Manager
void advanced_heap_init(void);
void* advanced_kmalloc(size_t size);
void advanced_kfree(void *ptr);
void* advanced_krealloc(void *ptr, size_t new_size);
void* kmalloc_aligned(size_t size, size_t alignment);
void* kcalloc(size_t num, size_t size);
void heap_print_stats(void);
void heap_test_advanced(void);

// Interrupt Handling
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void idt_flush(uint32_t);

// Keyboard Driver
void keyboard_init(void);
char keyboard_getchar(void);
extern void keyboard_handler(void);

// Port I/O
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// String functions
size_t strlen(const char *str);
void *memset(void *ptr, int value, size_t size);
void *memcpy(void *dest, const void *src, size_t size);
int strcmp(const char *str1, const char *str2);

// VGA Colors
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

// Process Management
void process_init(void);
void scheduler_init(void);
void schedule(void);

// System Calls
void syscall_init(void);

#endif // KERNEL_H
