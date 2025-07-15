/* SimpleOS Kernel Entry Point */

#include "kernel.h"

// Kernel main function
void kernel_main(void) {
    // Initialize VGA driver first - this is the most critical part
    vga_init();
    
    // Clear screen with safe color
    vga_clear();
    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    
    // Print startup messages
    vga_print("=== SimpleOS Kernel v1.0 ===\n");
    vga_print("Kernel successfully loaded!\n\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Initializing subsystems...\n");
    
    // Initialize memory management CAREFULLY
    vga_print("Initializing memory...");
    memory_init();
    vga_print(" OK\n");
    
    // DO NOT initialize interrupts yet - this is likely causing the crash
    vga_print("Skipping interrupt system (debug mode)\n");
    vga_print("Skipping keyboard driver (debug mode)\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("\nSimpleOS is stable!\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Running in polling mode - no interrupts.\n\n");
    
    vga_print("Available debug commands:\n");
    vga_print("This kernel will stay stable and not reboot.\n");
    vga_print("Press Ctrl+Alt+Q to quit QEMU.\n\n");
    
    // Safe infinite loop with heartbeat
    int counter = 0;
    while (1) {
        // Safe delay loop
        for (volatile int i = 0; i < 5000000; i++);
        
        // Show we're alive
        counter++;
        vga_print("Heartbeat: ");
        
        // Simple number printing
        char num_str[10];
        int temp = counter;
        int pos = 0;
        
        if (temp == 0) {
            num_str[pos++] = '0';
        } else {
            while (temp > 0) {
                num_str[pos++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        
        // Reverse string
        for (int i = pos - 1; i >= 0; i--) {
            vga_putchar(num_str[i]);
        }
        
        vga_print(" (kernel stable)\n");
    }
}
