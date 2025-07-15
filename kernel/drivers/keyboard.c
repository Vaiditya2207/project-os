#include "keyboard.h"
#include "../kernel.h"

// Global keyboard state
static keyboard_state_t kb_state = {0};

void keyboard_init(void) {
    kb_state.last_scancode = 0;
    kb_state.enter_cooldown = 0;
    kb_state.buffer_pos = 0;
    
    // Initialize key repeat state
    kb_state.held_key = 0;
    kb_state.key_hold_time = 0;
    kb_state.repeat_delay = 0;
    kb_state.held_key_char = 0;
    
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
        
        // Handle key releases (scancode >= 0x80)
        if (scancode >= 0x80) {
            uint8_t released_key = scancode - 0x80;
            if (released_key == kb_state.held_key) {
                // Key was released, stop repeat
                kb_state.held_key = 0;
                kb_state.key_hold_time = 0;
                kb_state.repeat_delay = 0;
                kb_state.held_key_char = 0;
            }
            kb_state.last_scancode = 0;
            continue;
        }
        
        // Process key presses
        if (scancode > 0 && scancode < 0x80) {
            // If this is the same key as before, handle repeat logic
            if (scancode == kb_state.held_key && kb_state.held_key != 0) {
                // Key is being held, increment hold time
                kb_state.key_hold_time++;
                
                if (kb_state.repeat_delay > 0) {
                    kb_state.repeat_delay--;
                } else if (kb_state.held_key_char != 0) {
                    // Time to repeat!
                    vga_print("\b"); // Remove cursor
                    
                    if (kb_state.held_key == BACKSPACE_SCANCODE) {
                        // Repeat backspace
                        if (kb_state.buffer_pos > 0) {
                            kb_state.buffer_pos--;
                            vga_print("\b ");
                            vga_print("\b");
                        }
                    } else if (kb_state.held_key_char != '\b' && kb_state.buffer_pos < 255) {
                        // Repeat normal character
                        kb_state.input_buffer[kb_state.buffer_pos++] = kb_state.held_key_char;
                        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                        vga_putchar(kb_state.held_key_char);
                        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    }
                    
                    vga_putchar('_'); // Show cursor again
                    
                    // Progressive acceleration: faster repeat rate after holding longer
                    if (kb_state.key_hold_time > KEY_REPEAT_ACCELERATION_THRESHOLD) {
                        kb_state.repeat_delay = KEY_REPEAT_FAST_RATE; // Super fast repeat
                    } else {
                        kb_state.repeat_delay = KEY_REPEAT_RATE; // Normal repeat rate
                    }
                }
            } else if (scancode != kb_state.last_scancode) {
                // New key pressed
                kb_state.last_scancode = scancode;
                kb_state.held_key = scancode;
                kb_state.key_hold_time = 0;
                kb_state.repeat_delay = KEY_REPEAT_INITIAL_DELAY;
                
                // Remove cursor
                vga_print("\b");
                
                // Handle Enter key with cooldown protection
                if (scancode == ENTER_SCANCODE) {
                    if (kb_state.enter_cooldown == 0) {
                        vga_print("\n");
                        kb_state.input_buffer[kb_state.buffer_pos] = '\0';
                        
                        // Clear keyboard buffer
                        keyboard_clear_buffer();
                        
                        // Set cooldown for Enter key
                        kb_state.enter_cooldown = ENTER_COOLDOWN_CYCLES;
                        kb_state.last_scancode = 0;
                        kb_state.held_key = 0; // Don't repeat Enter
                        return kb_state.input_buffer;
                    } else {
                        // Enter is in cooldown, ignore
                        vga_putchar('_');
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
                    kb_state.held_key_char = '\b'; // Set for repeat
                    vga_putchar('_');
                    continue;
                }
                
                // Handle normal character keys
                char key = scancode_to_ascii(scancode);
                if (key != 0 && kb_state.buffer_pos < 255) {
                    kb_state.input_buffer[kb_state.buffer_pos++] = key;
                    kb_state.held_key_char = key; // Store for repeat
                    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                    vga_putchar(key);
                    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                } else {
                    kb_state.held_key_char = 0; // Don't repeat unknown keys
                }
                
                // Show cursor again
                vga_putchar('_');
            }
        } else if (scancode == 0) {
            // No key activity, check if we should continue repeating
            if (kb_state.held_key != 0 && kb_state.held_key_char != 0) {
                kb_state.key_hold_time++;
                
                if (kb_state.repeat_delay > 0) {
                    kb_state.repeat_delay--;
                } else {
                    // Time to repeat!
                    vga_print("\b"); // Remove cursor
                    
                    if (kb_state.held_key == BACKSPACE_SCANCODE) {
                        // Repeat backspace
                        if (kb_state.buffer_pos > 0) {
                            kb_state.buffer_pos--;
                            vga_print("\b ");
                            vga_print("\b");
                        }
                    } else if (kb_state.held_key_char != '\b' && kb_state.buffer_pos < 255) {
                        // Repeat normal character
                        kb_state.input_buffer[kb_state.buffer_pos++] = kb_state.held_key_char;
                        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                        vga_putchar(kb_state.held_key_char);
                        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    }
                    
                    vga_putchar('_'); // Show cursor again
                    
                    // Progressive acceleration: faster repeat rate after holding longer
                    if (kb_state.key_hold_time > KEY_REPEAT_ACCELERATION_THRESHOLD) {
                        kb_state.repeat_delay = KEY_REPEAT_FAST_RATE; // Super fast repeat
                    } else {
                        kb_state.repeat_delay = KEY_REPEAT_RATE; // Normal repeat rate
                    }
                }
            }
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
