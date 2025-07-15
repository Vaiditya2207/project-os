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
    
    // Title banner
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("=====================================\n");
    vga_print("         Welcome to SimpleOS        \n");
    vga_print("=====================================\n\n");
    
    // System information
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("SimpleOS v2.0 - Phase 1 Implementation\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("A custom operating system built from scratch\n\n");
    
    // System status
    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
    vga_print("System Status:\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("  [OK] VGA Text Driver\n");
    vga_print("  [OK] Memory Management\n");
    vga_print("  [OK] Interrupt Tables\n");
    vga_print("  [OK] Interactive Shell\n\n");
    
    // Features
    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_print("Available Features:\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("  * Custom bootloader\n");
    vga_print("  * Protected mode kernel\n");
    vga_print("  * VGA text mode graphics\n");
    vga_print("  * Basic command interface\n");
    vga_print("  * Memory management\n\n");
    
    // Instructions
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("Welcome! Type 'help' for available commands.\n");
    vga_print("This OS was built entirely from assembly and C.\n\n");
    
    // Delay to let user read
    for (volatile int i = 0; i < 50000000; i++);
}

void interactive_shell(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Starting SimpleOS shell...\n\n");
    
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
    vga_print("SimpleOS");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("$ ");
}

char* get_input(void) {
    // Simulate user input - in real implementation this would read from keyboard
    // For now, we'll cycle through demo commands
    static int demo_command = 0;
    static char demo_commands[][20] = {
        "help",
        "about",
        "status",
        "memory",
        "clear",
        "version"
    };
    
    // Add delay to simulate typing
    for (volatile int i = 0; i < 30000000; i++);
    
    // Copy demo command to buffer
    string_copy(input_buffer, demo_commands[demo_command]);
    
    // Print what user "typed"
    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
    vga_print(input_buffer);
    vga_print("\n");
    
    demo_command = (demo_command + 1) % 6;
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
        vga_clear();
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Screen cleared!\n");
        
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
