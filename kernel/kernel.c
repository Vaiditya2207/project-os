/* SimpleOS Kernel Entry Point - Phase 1 */

#include "kernel.h"

// Forward declarations
void show_welcome_screen(void);
void interactive_shell(void);
void process_command(char *command);
void print_prompt(void);
char* get_input(void);
int string_compare(const char *str1, const char *str2);
void string_copy(char *dest, const char *src);
int string_length(const char *str);

// Simple command buffer
static char input_buffer[256];
static int buffer_pos = 0;

// Kernel main function
void kernel_main(void) {
    // Initialize VGA driver first
    vga_init();
    vga_clear();
    
    // Initialize subsystems quietly
    memory_init();
    idt_init();
    
    // Show welcome screen
    show_welcome_screen();
    
    // Start interactive shell
    interactive_shell();
}

void show_welcome_screen(void) {
    vga_clear();
    
    // Simple title
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("========= SimpleOS =========\n\n");
    
    // Just list command names
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Commands: help, about, status, memory, version, clear\n\n");
    
    // Brief instruction
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("Try any command!\n\n");
}

void interactive_shell(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Starting interactive shell...\n\n");
    
    while (1) {
        print_prompt();
        char *command = get_input();
        
        if (string_length(command) > 0) {
            process_command(command);
        }
        
        // Clear buffer
        buffer_pos = 0;
        for (int i = 0; i < 256; i++) {
            input_buffer[i] = 0;
        }
    }
}

void print_prompt(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("SimpleOS-v1.1");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("> ");
}

char* get_input(void) {
    // Show cursor at the end of the prompt line
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putchar('_');
    
    // Wait indefinitely - no automatic cycling
    while (1) {
        // Infinite loop - waiting for real keyboard input
        // In a real OS, this would wait for keyboard interrupt
        for (volatile int i = 0; i < 50000000; i++);
    }
    
    return input_buffer;
}

void process_command(char *command) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    if (string_compare(command, "help")) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Available Commands:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  help     - Show this help message\n");
        vga_print("  about    - About SimpleOS\n");
        vga_print("  status   - System status\n");
        vga_print("  memory   - Memory information\n");
        vga_print("  clear    - Clear screen\n");
        vga_print("  version  - Show version info\n");
        
    } else if (string_compare(command, "about")) {
        vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
        vga_print("About SimpleOS:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("SimpleOS is a custom operating system built\n");
        vga_print("entirely from scratch using x86 assembly and C.\n");
        vga_print("It features a custom bootloader, protected mode\n");
        vga_print("kernel, and interactive command interface.\n");
        
    } else if (string_compare(command, "status")) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("System Status:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  CPU Mode: 32-bit Protected Mode\n");
        vga_print("  Memory: Initialized\n");
        vga_print("  VGA: 80x25 Text Mode\n");
        vga_print("  Interrupts: Disabled (safe mode)\n");
        vga_print("  Shell: Active\n");
        
    } else if (string_compare(command, "memory")) {
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_print("Memory Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  Kernel loaded at: 0x1000\n");
        vga_print("  Stack pointer: 0x90000\n");
        vga_print("  VGA buffer: 0xB8000\n");
        vga_print("  Available RAM: 128MB (QEMU)\n");
        
    } else if (string_compare(command, "clear")) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Clear command received.\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("(Screen clearing disabled in demo mode)\n");
        
    } else if (string_compare(command, "version")) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("SimpleOS Version Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  Kernel: v2.0 Phase 1\n");
        vga_print("  Bootloader: v1.0\n");
        vga_print("  Architecture: x86 (i386)\n");
        vga_print("  Build: Custom from scratch\n");
        
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_print("Unknown command: ");
        vga_print(command);
        vga_print("\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("Type 'help' for available commands.\n");
    }
    
    vga_print("\n");
}

// Utility functions
int string_compare(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] && str2[i]) {
        if (str1[i] != str2[i]) {
            return 0;
        }
        i++;
    }
    return str1[i] == str2[i];
}

void string_copy(char *dest, const char *src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

int string_length(const char *str) {
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}
