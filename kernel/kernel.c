/* SimpleOS Kernel Entry Point - v1.2.1 with Process Management */

#include <stdint.h> // Add stdint.h include for uint16_t and uint8_t types

#include "kernel.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "proc/process.h"
#include "syscalls.h"
#include "mem/vmm.h"
#include "mem/advanced_heap.h"

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
int string_starts_with(const char *str, const char *prefix);
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

    vga_print("Initializing physical memory manager...\n");
    pmm_init();

    vga_print("Initializing virtual memory manager...\n");
    vmm_init();

    vga_print("Initializing advanced heap manager...\n");
    advanced_heap_init();

    vga_print("Initializing IDT...\n");
    idt_init();

    vga_print("Initializing timer...\n");
    timer_init(); // Initialize timer for preemptive scheduling

    vga_print("Initializing keyboard...\n");
    keyboard_init(); // Initialize keyboard system

    // Initialize process management with multitasking support
    vga_print("Initializing process management...\n");
    process_init();
    vga_print("Initializing scheduler...\n");
    scheduler_init();

    vga_print("Enabling process execution...\n");
    enable_process_execution(); // Enable real multitasking

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
    vga_print("SimpleOS-v1.2.2");
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
        vga_print("  memstat  - Physical memory statistics\n");
        vga_print("  memtest  - Test physical memory allocation\n");
        vga_print("  vmstat   - Virtual memory statistics\n");
        vga_print("  vmtest   - Test virtual memory allocation\n");
        vga_print("  prottest - Test memory protection features\n");
        vga_print("  heapstat - Advanced heap statistics\n");
        vga_print("  heaptest - Test advanced heap features\n");
        vga_print("  clear    - Clear screen\n");
        vga_print("  version  - Show version info\n");
        vga_print("  keytest  - Test enhanced keyboard features\n");
        vga_print("  ps       - List all processes\n");
        vga_print("  proc     - Current process info\n");
        vga_print("  spawn <name>    - Create process with name\n");
        vga_print("  pkill <pid>     - Kill process by PID\n");
        vga_print("                    (Note: PID 1 kernel_idle is protected)\n");
        vga_print("  pstatus <pid> <status> - Set process status (READY/PAUSED/WAITING)\n");
        vga_print("  fork            - Fork current process\n");
        vga_print("  exec <prog>     - Execute program\n");
        vga_print("  getpid          - Get current process ID\n");
        vga_print("  schedule        - Trigger manual scheduler\n");
        vga_print("  sysinfo         - Show system protection info\n");
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
        if (used_kb < 10)
        {
            vga_putchar('0' + used_kb);
        }
        else
        {
            vga_putchar('0' + (used_kb / 100));
            vga_putchar('0' + ((used_kb / 10) % 10));
            vga_putchar('0' + (used_kb % 10));
        }
        vga_print("KB / ");
        if (total_kb < 10)
        {
            vga_putchar('0' + total_kb);
        }
        else
        {
            vga_putchar('0' + (total_kb / 100));
            vga_putchar('0' + ((total_kb / 10) % 10));
            vga_putchar('0' + (total_kb % 10));
        }
        vga_print("KB\n");
    }
    else if (string_compare(command, "memstat"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Physical Memory Statistics:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        pmm_print_stats();
    }
    else if (string_compare(command, "memtest"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Running Physical Memory Tests:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        pmm_test_allocation();
    }
    else if (string_compare(command, "vmstat"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Virtual Memory Statistics:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vmm_print_stats();
    }
    else if (string_compare(command, "vmtest"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Running Virtual Memory Tests:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vmm_test_paging();
    }
    else if (string_compare(command, "prottest"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Testing Memory Protection:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        // Test user directory creation
        void *user_dir = vmm_create_user_directory();
        if (user_dir) {
            vga_print("  User page directory created: 0x");
            vga_print_hex((uint32_t)user_dir);
            vga_print("\n");
            
            // Test user stack setup
            if (vmm_setup_user_stack(user_dir, 0xBFFFF000, 0x1000)) {
                vga_print("  User stack setup: PASSED\n");
            } else {
                vga_print("  User stack setup: FAILED\n");
            }
            
            // Test user heap setup
            if (vmm_setup_user_heap(user_dir, 0x10000000, 0x1000)) {
                vga_print("  User heap setup: PASSED\n");
            } else {
                vga_print("  User heap setup: FAILED\n");
            }
            
            // Test address validation
            bool valid = vmm_is_address_valid(user_dir, 0xBFFFE000, true, true);
            vga_print("  User stack access validation: ");
            vga_print(valid ? "PASSED\n" : "FAILED\n");
            
            // Test kernel address protection
            valid = vmm_is_address_valid(user_dir, 0xC0000000, false, true);
            vga_print("  Kernel space protection: ");
            vga_print(!valid ? "PASSED\n" : "FAILED\n");
            
        } else {
            vga_print("  Failed to create user directory\n");
        }
    }
    else if (string_compare(command, "heapstat"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Advanced Heap Statistics:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        heap_print_stats();
    }
    else if (string_compare(command, "heaptest"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Running Advanced Heap Tests:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        heap_test_advanced();
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
        vga_print("  Kernel: v1.2.2 - Enhanced Keyboard Driver with Special Characters\n");
        vga_print("  Features: Caps Lock, Shift, Tab, Special chars, Full ASCII support\n");
        vga_print("  Previous: v1.2.1 - Full Process Management with Context Switching\n");
        vga_print("  Bootloader: v1.0\n");
        vga_print("  Architecture: x86 (i386)\n");
        vga_print("  Build: Custom from scratch\n");
    }
    else if (string_compare(command, "keytest"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Enhanced Keyboard Test Mode:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("Test these features:\n");
        vga_print("  - Letters: abcdefghijklmnopqrstuvwxyz\n");
        vga_print("  - Numbers: 1234567890\n");
        vga_print("  - Shift+Numbers: !@#$%^&*()\n");
        vga_print("  - Special chars: []{}\\|;:'\"<>,./?`~-=_+\n");
        vga_print("  - Caps Lock (toggle with Caps Lock key)\n");
        vga_print("  - Tab (inserts 4 spaces)\n");
        vga_print("  - Backspace (deletes characters)\n");
        vga_print("\nType anything to test, 'exit' to return:\n");

        while (1)
        {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_print("KeyTest> ");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            char *test_input = keyboard_get_input();

            if (string_compare(test_input, "exit"))
            {
                vga_print("Keyboard test completed!\n");
                break;
            }

            vga_print("You typed: '");
            vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
            vga_print(test_input);
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            vga_print("' (length: ");

            // Print length
            int len = string_length(test_input);
            if (len < 10)
            {
                vga_putchar('0' + len);
            }
            else
            {
                vga_putchar('0' + (len / 10));
                vga_putchar('0' + (len % 10));
            }
            vga_print(")\n");
        }
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

        if (current_process)
        {
            vga_print("  PID: ");
            vga_print_hex(current_process->pid);
            vga_print("\n  Name: ");
            vga_print(current_process->name);
            vga_print("\n  State: ");

            switch (current_process->state)
            {
            case PROCESS_READY:
                vga_print("READY");
                break;
            case PROCESS_RUNNING:
                vga_print("RUNNING");
                break;
            case PROCESS_BLOCKED:
                vga_print("BLOCKED");
                break;
            case PROCESS_TERMINATED:
                vga_print("TERMINATED");
                break;
            default:
                vga_print("UNKNOWN");
            }

            vga_print("\n  Priority: ");
            switch (current_process->priority)
            {
            case PRIORITY_HIGH:
                vga_print("HIGH");
                break;
            case PRIORITY_NORMAL:
                vga_print("NORMAL");
                break;
            case PRIORITY_LOW:
                vga_print("LOW");
                break;
            }
            vga_print("\n");
        }
        else
        {
            vga_print("No current process (kernel mode)\n");
        }
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

        int result = process_kill_by_pid(pid);
        if (result == PROCESS_SUCCESS)
        {
            vga_print("Process killed successfully.\n");
        }
        else if (result == PROCESS_PROTECTED)
        {
            // Error message already printed by process_kill_by_pid
            // Just set the color back to normal since error was already shown
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
        else if (result == PROCESS_NOT_FOUND)
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Process not found!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Failed to kill process!\n");
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
    else if (string_compare(command, "fork"))
    {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("Forking current process...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

        uint32_t child_pid = sys_fork();
        if (child_pid > 0)
        {
            vga_print("Child process created with PID: ");
            vga_print_hex(child_pid);
            vga_print("\n");
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Fork failed!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }
    else if (string_starts_with(command, "exec "))
    {
        // Parse program name from command
        char *program_name = command + 5; // Skip "exec "

        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("Executing program: ");
        vga_print(program_name);
        vga_print("\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

        int result = sys_exec(program_name, NULL);
        if (result == 0)
        {
            vga_print("Program executed successfully\n");
        }
        else
        {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Exec failed!\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }
    else if (string_compare(command, "getpid"))
    {
        uint32_t current_pid = sys_getpid();
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Current process PID: ");
        vga_print_hex(current_pid);
        vga_print("\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    }
    else if (string_compare(command, "schedule"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("Triggering manual scheduler...\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        schedule();
        vga_print("Scheduler executed\n");
    }
    else if (string_compare(command, "sysinfo"))
    {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("System Protection Information:\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_print("  Protected Processes:\n");
        vga_print("    PID 1 (kernel_idle) - Cannot be killed\n");
        vga_print("    Current kernel process - System critical\n");
        vga_print("\n");
        vga_print("  Security Features:\n");
        vga_print("    - Kernel process protection enabled\n");
        vga_print("    - Critical PID protection (PID 1)\n");
        vga_print("    - Memory cleanup on process termination\n");
        vga_print("    - Process state validation\n");
        vga_print("\n");
        vga_print("  Process Management:\n");
        vga_print("    - Maximum processes: 256\n");
        vga_print("    - Static memory allocation for safety\n");
        vga_print("    - Context switching with timer interrupts\n");
        vga_print("    - Preemptive scheduling at 100Hz\n");
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

int string_starts_with(const char *str, const char *prefix)
{
    int i = 0;
    while (prefix[i])
    {
        if (str[i] != prefix[i])
        {
            return 0;
        }
        i++;
    }
    return 1;
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
    while (command[i] == ' ')
        i++;

    // Parse command
    while (command[i] && command[i] != ' ')
    {
        cmd[j++] = command[i++];
    }
    cmd[j] = '\0';

    if (!cmd[0])
        return 0; // No command found
    arg_count = 1;

    // Skip spaces
    while (command[i] == ' ')
        i++;

    // Parse first argument
    if (command[i])
    {
        j = 0;
        while (command[i] && command[i] != ' ')
        {
            arg1[j++] = command[i++];
        }
        arg1[j] = '\0';
        if (arg1[0])
            arg_count = 2;

        // Skip spaces
        while (command[i] == ' ')
            i++;

        // Parse second argument
        if (command[i])
        {
            j = 0;
            while (command[i] && command[i] != ' ')
            {
                arg2[j++] = command[i++];
            }
            arg2[j] = '\0';
            if (arg2[0])
                arg_count = 3;
        }
    }

    return arg_count;
}
