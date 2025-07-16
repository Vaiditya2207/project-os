/* SimpleOS Kernel Entry Point - Phase 1 */

#include <stdint.h> // Add stdint.h include for uint16_t and uint8_t types

#include "kernel.h"
#include "drivers/keyboard.h"
#include "proc/process.h" // Re-enabling for debugging

// Simple serial output for debugging
void serial_write_char(char c)
{
    // Write to COM1 (0x3F8)
    while (!(inb(0x3F8 + 5) & 0x20))
        ; // Wait for transmitter ready
    outb(0x3F8, c);
}

void serial_write_string(const char *str)
{
    while (*str)
    {
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
int parse_arguments(const char *command, char *cmd, char *arg1, char *arg2);
process_t *process_create_test(const char *name, void *entry_point, process_priority_t priority);

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
    process_init();  // Re-enable just the init
    vga_print("Initializing scheduler...\n");
    scheduler_init();  // Re-enable just the init

    vga_print("Showing welcome screen...\n");
    // Show welcome screen
    show_welcome_screen();

    vga_print("DEBUG: Welcome screen shown, starting shell...\n");

    // Small delay to let welcome screen render
    for (volatile int i = 0; i < 1000000; i++)
        ;

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
    vga_print("Process:  ps, proc, spawn <name>, pkill <pid>, pstatus <pid> <status>\n");
    vga_print("\n");

    // Brief instruction
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("Try any command!\n\n");

    vga_print("DEBUG: Welcome screen completed\n");
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
        vga_print("  ps       - List all processes\n");
        vga_print("  proc     - Current process info\n");
        vga_print("  spawn <name>    - Create process with name\n");
        vga_print("  pkill <pid>     - Kill process by PID\n");
        vga_print("  pstatus <pid> <status> - Set process status (READY/PAUSED/WAITING)\n");
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
        vga_print("  Heap start: 0x100000 (1MB)\n");
        vga_print("  Heap size: 1MB\n");
        
        // Calculate heap usage
        extern uint8_t *heap_start, *heap_current, *heap_end;
        uint32_t used = (uint32_t)heap_current - (uint32_t)heap_start;
        uint32_t total = (uint32_t)heap_end - (uint32_t)heap_start;
        uint32_t used_kb = used / 1024;
        uint32_t total_kb = total / 1024;
        
        vga_print("  Heap used: ");
        if (used_kb < 10) {
            vga_putchar('0' + used_kb);
        } else {
            vga_putchar('0' + (used_kb / 100));
            vga_putchar('0' + ((used_kb / 10) % 10));
            vga_putchar('0' + (used_kb % 10));
        }
        vga_print("KB / ");
        if (total_kb < 10) {
            vga_putchar('0' + total_kb);
        } else {
            vga_putchar('0' + (total_kb / 100));
            vga_putchar('0' + ((total_kb / 10) % 10));
            vga_putchar('0' + (total_kb % 10));
        }
        vga_print("KB\n");
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
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_print("Process management temporarily disabled.\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    }
    else if (command[0] == 's' && command[1] == 'p' && command[2] == 'a' && command[3] == 'w' && command[4] == 'n' && (command[5] == ' ' || command[5] == '\0'))
    {
        // Parse arguments for spawn command
        char cmd[64], arg1[64], arg2[64];
        int argc = parse_arguments(command, cmd, arg1, arg2);
        
        if (argc < 2)
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Usage: spawn <name>\n");
            vga_print("Example: spawn myprocess\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            return;
        }
        
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("Creating process '");
        vga_print(arg1);
        vga_print("'...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

        // Use test function that doesn't call kmalloc
        process_t *new_process = process_create_test(arg1, (void *)demo_counter_process, PRIORITY_NORMAL);
        
        if (new_process)
        {
            vga_print("Process created successfully!\n");
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Failed to create process!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }
    else if (command[0] == 'p' && command[1] == 'k' && command[2] == 'i' && command[3] == 'l' && command[4] == 'l' && (command[5] == ' ' || command[5] == '\0'))
    {
        // Parse arguments for pkill command
        char cmd[64], arg1[64], arg2[64];
        int argc = parse_arguments(command, cmd, arg1, arg2);
        
        if (argc < 2)
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Usage: pkill <pid>\n");
            vga_print("Example: pkill 1\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            return;
        }
        
        // Simple PID parsing (supports 1-2 digits)
        uint32_t pid = 0;
        char *pidstr = arg1;
        while (*pidstr >= '0' && *pidstr <= '9')
        {
            pid = pid * 10 + (*pidstr - '0');
            pidstr++;
        }
        
        if (pid == 0 || *pidstr != '\0')
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Invalid PID format!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            return;
        }
        
        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
        vga_print("Killing process PID ");
        vga_print(arg1);
        vga_print("...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        if (process_kill_by_pid(pid))
        {
            vga_print("Process killed successfully.\n");
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Process not found!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }
    else if (command[0] == 'p' && command[1] == 's' && command[2] == 't' && command[3] == 'a' && command[4] == 't' && command[5] == 'u' && command[6] == 's' && (command[7] == ' ' || command[7] == '\0'))
    {
        // Parse arguments for pstatus command
        char cmd[64], arg1[64], arg2[64];
        int argc = parse_arguments(command, cmd, arg1, arg2);
        
        if (argc < 3)
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Usage: pstatus <pid> <status>\n");
            vga_print("Status options: READY, PAUSED, WAITING\n");
            vga_print("Example: pstatus 1 PAUSED\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            return;
        }
        
        // Parse PID
        uint32_t pid = 0;
        char *pidstr = arg1;
        while (*pidstr >= '0' && *pidstr <= '9')
        {
            pid = pid * 10 + (*pidstr - '0');
            pidstr++;
        }
        
        if (pid == 0 || *pidstr != '\0')
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Invalid PID format!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            return;
        }
        
        // Parse status
        process_state_t status;
        if (string_compare(arg2, "READY"))
        {
            status = PROCESS_READY;
        }
        else if (string_compare(arg2, "PAUSED"))
        {
            status = PROCESS_BLOCKED;
        }
        else if (string_compare(arg2, "WAITING"))
        {
            status = PROCESS_BLOCKED; // Use BLOCKED for both PAUSED and WAITING
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Invalid status! Use: READY, PAUSED, or WAITING\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            return;
        }
        
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Setting process PID ");
        vga_print(arg1);
        vga_print(" to ");
        vga_print(arg2);
        vga_print("...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        if (process_set_status(pid, status))
        {
            vga_print("Process status updated successfully.\n");
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Failed to update process status!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
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

// Parse command arguments - returns number of arguments found
int parse_arguments(const char *command, char *cmd, char *arg1, char *arg2)
{
    int i = 0, j = 0;
    int arg_count = 0;
    
    // Clear all output buffers
    cmd[0] = '\0';
    arg1[0] = '\0';
    arg2[0] = '\0';
    
    // Skip leading spaces
    while (command[i] == ' ') i++;
    
    // Parse command
    while (command[i] && command[i] != ' ')
    {
        cmd[j++] = command[i++];
    }
    cmd[j] = '\0';
    
    if (!cmd[0]) return 0; // No command found
    arg_count = 1;
    
    // Skip spaces
    while (command[i] == ' ') i++;
    
    // Parse first argument
    if (command[i])
    {
        j = 0;
        while (command[i] && command[i] != ' ')
        {
            arg1[j++] = command[i++];
        }
        arg1[j] = '\0';
        if (arg1[0]) arg_count = 2;
        
        // Skip spaces
        while (command[i] == ' ') i++;
        
        // Parse second argument
        if (command[i])
        {
            j = 0;
            while (command[i] && command[i] != ' ')
            {
                arg2[j++] = command[i++];
            }
            arg2[j] = '\0';
            if (arg2[0]) arg_count = 3;
        }
    }
    
    return arg_count;
}
