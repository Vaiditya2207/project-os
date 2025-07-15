/* Ultra-Simple Test Kernel */

// Simple VGA text output
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void simple_print(const char* str, int row) {
    volatile unsigned short* vga = (volatile unsigned short*)VGA_MEMORY;
    int col = 0;
    
    while (*str && col < VGA_WIDTH) {
        // White text on black background
        vga[row * VGA_WIDTH + col] = (unsigned short)*str | (0x07 << 8);
        str++;
        col++;
    }
}

void kernel_main(void) {
    // Clear screen
    volatile unsigned short* vga = (volatile unsigned short*)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (unsigned short)' ' | (0x07 << 8);
    }
    
    // Print test messages
    simple_print("SimpleOS - Ultra Simple Test Kernel", 0);
    simple_print("", 1);
    simple_print("If you can see this text, the kernel is working!", 2);
    simple_print("", 3);
    simple_print("VGA driver: OK", 4);
    simple_print("Memory access: OK", 5);
    simple_print("C code execution: OK", 6);
    simple_print("", 7);
    simple_print("Kernel is now in infinite loop - this is normal.", 8);
    simple_print("Press Ctrl+Alt+Q to quit QEMU.", 9);
    
    // Infinite loop
    while (1) {
        // Do nothing, just stay alive
        for (volatile int i = 0; i < 10000000; i++);
    }
}
