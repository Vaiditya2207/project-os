/* SimpleOS Kernel Entry Point - Phase 1 */

#include "kernel.h"
#include "drivers/keyboard.h"
// #include "proc/process.h"  // Temporarily disabled

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

    vga_print("SimpleOS Kernel Starting...\n");

    // Initialize subsystems quietly
    vga_print("Initializing memory...\n");
    memory_init();

    vga_print("Initializing IDT...\n");
    idt_init();

    vga_print("Initializing keyboard...\n");
    keyboard_init(); // Initialize new keyboard system

    // TEMPORARILY DISABLE process management to test basic kernel
    // vga_print("Initializing process management...\n");
    // process_init();
    // scheduler_init();

    vga_print("Showing welcome screen...\n");
    // Show welcome screen
    show_welcome_screen();

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
    vga_clear();

    // Simple title
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("========= SimpleOS =========\n\n");

    // Just list command names
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Commands: help, about, status, memory, version, clear\n");
    // vga_print("Process:  ps, proc, spawn, stress\n\n");
    vga_print("\n");

    // Brief instruction
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("Try any command!\n\n");
}

void interactive_shell(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Starting interactive shell...\n\n");

    while (1)
    {
        print_prompt();
        char *command = keyboard_get_input(); // Use new keyboard system

        if (string_length(command) > 0)
        {
            process_command(command);
        }
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
        // Process commands temporarily disabled
        // vga_print("  ps       - List all processes\n");
        // vga_print("  proc     - Current process info\n");
        // vga_print("  spawn    - Create demo processes\n");
        // vga_print("  stress   - Stress test (5 processes)\n");
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
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Clear command received.\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("(Screen clearing disabled in demo mode)\n");
    }
    else if (string_compare(command, "version"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("SimpleOS Version Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  Kernel: v1.1.2 - Stable Basic Kernel\n");
        vga_print("  Bootloader: v1.0\n");
        vga_print("  Architecture: x86 (i386)\n");
        vga_print("  Build: Custom from scratch\n");

        // Process management commands temporarily disabled
        // Will be re-enabled once process management is fixed
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
