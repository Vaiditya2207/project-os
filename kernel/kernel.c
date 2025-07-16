/* SimpleOS Kernel Entry Point - Phase 1 */

#include <stdint.h> // Add stdint.h include for uint16_t and uint8_t types

#include "kernel.h"
#include "drivers/keyboard.h"
#include "proc/process.h"  // Re-enabling for debugging

// Simple serial output for debugging
void serial_write_char(char c) {
    // Write to COM1 (0x3F8)
    while (!(inb(0x3F8 + 5) & 0x20));  // Wait for transmitter ready
    outb(0x3F8, c);
}

void serial_write_string(const char *str) {
    while (*str) {
        serial_write_char(*str++);
    }
}

// Forward declarations
void show_welcome_screen(void);
void interactive_shell(void);
void process_command(char *command);
void print_prompt(void);
int string_compare(const char *str1, const char *str2);
void string_copy(char *dest, const char *src);
int string_length(const char *str);

// Kernel main function
void kernel_main(void)
{
    // Initialize VGA driver first
    vga_init();
    vga_clear();

    serial_write_string("SERIAL: Kernel started\n");
    vga_print("SimpleOS Kernel Starting...\n");

    // Initialize subsystems quietly
    vga_print("Initializing memory...\n");
    memory_init();

    vga_print("Initializing IDT...\n");
    idt_init();

    vga_print("Initializing keyboard...\n");
    keyboard_init(); // Initialize new keyboard system

    // Re-enable process management with minimal functionality
    vga_print("Initializing process management...\n");
    process_init();
    vga_print("Initializing scheduler...\n");
    scheduler_init();

    vga_print("Showing welcome screen...\n");
    // Show welcome screen
    show_welcome_screen();

    vga_print("DEBUG: Welcome screen shown, starting shell...\n");
    
    // Small delay to let welcome screen render
    for (volatile int i = 0; i < 1000000; i++);
    
    // Start interactive shell
    interactive_shell();

    // Should never reach here, but just in case...
    while (1)
    {
        asm volatile("hlt");
    }
}

void show_welcome_screen(void)
{
    serial_write_string("SERIAL: Starting welcome screen function\n");
    vga_print("DEBUG: Starting welcome screen function\n");

    // Simple title
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("========= SimpleOS =========\n\n");

    // Just list command names
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Commands: help, about, status, memory, version, clear\n");
    vga_print("Process:  ps, proc, spawn, stress\n");
    vga_print("\n");

    // Brief instruction
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("Try any command!\n\n");
    
    vga_print("DEBUG: Welcome screen completed\n");
}

void interactive_shell(void)
{
    char input_buffer[256];
    int buffer_index = 0;
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Starting interactive shell...\n\n");
    
    // Clear keyboard buffer before starting
    keyboard_clear_buffer();
    
    // Small delay to let things settle
    for (volatile int i = 0; i < 500000; i++);

    // Show first prompt
    print_prompt();

    while (1)
    {
        char c = keyboard_getchar();
        
        if (c != 0) // Valid character received
        {
            if (c == '\n') // Enter key pressed
            {
                vga_print("\n");
                
                if (buffer_index > 0)
                {
                    input_buffer[buffer_index] = '\0'; // Null terminate
                    process_command(input_buffer);
                    buffer_index = 0; // Reset buffer
                }
                
                print_prompt();
            }
            else if (c == '\b') // Backspace
            {
                if (buffer_index > 0)
                {
                    buffer_index--;
                    vga_print("\b \b"); // Move back, print space, move back again
                }
            }
            else if (c >= 32 && c <= 126 && buffer_index < 255) // Printable characters
            {
                input_buffer[buffer_index] = c;
                buffer_index++;
                
                // Echo the character
                char temp_str[2];
                temp_str[0] = c;
                temp_str[1] = '\0';
                vga_print(temp_str);
            }
        }
        
        // Small delay to prevent excessive CPU usage
        for (volatile int i = 0; i < 1000; i++);
    }
}

void print_prompt(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("SimpleOS-v1.1.2");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("> ");
}

void process_command(char *command)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    if (string_compare(command, "help"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Available Commands:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  help     - Show this help message\n");
        vga_print("  about    - About SimpleOS\n");
        vga_print("  status   - System status\n");
        vga_print("  memory   - Memory information\n");
        vga_print("  clear    - Clear screen\n");
        vga_print("  version  - Show version info\n");
        vga_print("  ps       - List all processes\n");
        vga_print("  proc     - Current process info\n");
        vga_print("  spawn    - Create demo processes\n");
        vga_print("  stress   - Stress test (5 processes)\n");
    }
    else if (string_compare(command, "about"))
    {
        vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
        vga_print("About SimpleOS:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("SimpleOS is a custom operating system built\n");
        vga_print("entirely from scratch using x86 assembly and C.\n");
        vga_print("It features a custom bootloader, protected mode\n");
        vga_print("kernel, and interactive command interface.\n");
    }
    else if (string_compare(command, "status"))
    {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("System Status:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  CPU Mode: 32-bit Protected Mode\n");
        vga_print("  Memory: Initialized\n");
        vga_print("  VGA: 80x25 Text Mode\n");
        vga_print("  Interrupts: Disabled (safe mode)\n");
        vga_print("  Shell: Active\n");
    }
    else if (string_compare(command, "memory"))
    {
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_print("Memory Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  Kernel loaded at: 0x1000\n");
        vga_print("  Stack pointer: 0x90000\n");
        vga_print("  VGA buffer: 0xB8000\n");
        vga_print("  Available RAM: 128MB (QEMU)\n");
    }
    else if (string_compare(command, "clear"))
    {
        vga_clear();
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Screen cleared.\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    }
    else if (string_compare(command, "version"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("SimpleOS Version Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  Kernel: v1.2.0 - Process Management RESTORED\n");
        vga_print("  Bootloader: v1.0\n");
        vga_print("  Architecture: x86 (i386)\n");
        vga_print("  Build: Custom from scratch\n");
    }
    else if (string_compare(command, "ps"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Process List:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        process_list_all();
    }
    else if (string_compare(command, "proc"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Current Process Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        process_t *current = scheduler_get_current();
        if (current) {
            process_print_info(current);
        } else {
            vga_print("No current process (kernel mode)\n");
        }
    }
    else if (string_compare(command, "spawn"))
    {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("Creating demo processes...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        create_demo_processes();
    }
    else if (string_compare(command, "stress"))
    {
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_print("Running stress test (5 processes)...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        process_stress_test();
    }
    else
    {
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
int string_compare(const char *str1, const char *str2)
{
    int i = 0;
    while (str1[i] && str2[i])
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
        i++;
    }
    return str1[i] == str2[i];
}

void string_copy(char *dest, const char *src)
{
    int i = 0;
    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

int string_length(const char *str)
{
    int len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}
