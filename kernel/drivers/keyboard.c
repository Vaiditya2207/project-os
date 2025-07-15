#include "keyboard.h"
#include "../kernel.h"

// Global keyboard state
static keyboard_state_t kb_state = {0};

void keyboard_init(void) {
    kb_state.last_scancode = 0;
    kb_state.enter_cooldown = 0;
    kb_state.buffer_pos = 0;
    
    // Clear input buffer
    for (int i = 0; i < 256; i++) {
        kb_state.input_buffer[i] = 0;
    }
}

char scancode_to_ascii(uint8_t scancode) {
    // Scancode to ASCII mapping table
    switch (scancode) {
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x39: return ' '; // Space
        default: return 0;     // Unknown key
    }
}

void keyboard_clear_buffer(void) {
    // Drain keyboard buffer
    for (int clear_attempts = 0; clear_attempts < 3; clear_attempts++) {
        uint8_t dummy = inb(KEYBOARD_DATA_PORT);
        (void)dummy;
        for (volatile int j = 0; j < BUFFER_CLEAR_DELAY; j++);
    }
}

char* keyboard_get_input(void) {
    // Reset buffer position
    kb_state.buffer_pos = 0;
    
    // Show cursor
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putchar('_');
    
    while (1) {
        // Poll keyboard port for input
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);
        
        // Reduce Enter cooldown over time
        if (kb_state.enter_cooldown > 0) {
            kb_state.enter_cooldown--;
        }
        
        // Only process actual key presses (not releases or noise)
        if (scancode > 0 && scancode < 0x80 && scancode != kb_state.last_scancode) {
            kb_state.last_scancode = scancode;
            
            // Remove cursor
            vga_print("\b");
            
            // Handle Enter key with cooldown protection
            if (scancode == ENTER_SCANCODE) {
                // Only process Enter if cooldown has expired
                if (kb_state.enter_cooldown == 0) {
                    vga_print("\n");
                    kb_state.input_buffer[kb_state.buffer_pos] = '\0';
                    
                    // Reduced delay to prevent immediate re-trigger
                    for (volatile int i = 0; i < 15000; i++);
                    
                    // Clear keyboard buffer
                    keyboard_clear_buffer();
                    
                    // Set reduced cooldown for Enter key
                    kb_state.enter_cooldown = ENTER_COOLDOWN_CYCLES;
                    kb_state.last_scancode = 0;
                    return kb_state.input_buffer;
                } else {
                    // Enter is in cooldown, ignore this press
                    vga_putchar('_'); // Put cursor back
                    continue;
                }
            }
            
            // Handle backspace
            if (scancode == BACKSPACE_SCANCODE) {
                if (kb_state.buffer_pos > 0) {
                    kb_state.buffer_pos--;
                    vga_print("\b ");
                    vga_print("\b");
                }
                vga_putchar('_');
                continue;
            }
            
            // Handle all other keys
            char key = scancode_to_ascii(scancode);
            
            // Add character to buffer (normal keys work regardless of Enter cooldown)
            if (key != 0 && kb_state.buffer_pos < 255) {
                kb_state.input_buffer[kb_state.buffer_pos++] = key;
                vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                vga_putchar(key);
                vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            }
            
            // Show cursor again
            vga_putchar('_');
            
            // Reduced delay for other keys
            for (volatile int i = 0; i < KEY_DELAY_CYCLES; i++);
        } 
        // Reset tracking on key release
        else if (scancode >= 0x80) {
            kb_state.last_scancode = 0;
        }
        
        // Light polling delay
        for (volatile int i = 0; i < 5000; i++);
    }
    
    return kb_state.input_buffer;
}

// Legacy functions for compatibility
char keyboard_getchar(void) {
    return 0;
}

void keyboard_handler_internal(void) {
    // Legacy interrupt handler - disabled
}
