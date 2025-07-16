#include "keyboard.h"
#include "../kernel.h"

// Global keyboard state
static keyboard_state_t kb_state = {0};

void keyboard_init(void)
{
    kb_state.last_scancode = 0;
    kb_state.enter_cooldown = 0;
    kb_state.buffer_pos = 0;

    // Initialize key repeat state
    kb_state.held_key = 0;
    kb_state.key_hold_time = 0;
    kb_state.repeat_delay = 0;
    kb_state.held_key_char = 0;

    // Clear input buffer
    for (int i = 0; i < 256; i++)
    {
        kb_state.input_buffer[i] = 0;
    }
    
    // IMPORTANT: Drain keyboard buffer completely to remove any garbage
    for (int i = 0; i < 100; i++) {
        inb(KEYBOARD_DATA_PORT); // Clear any pending data
        for (volatile int j = 0; j < 1000; j++); // Small delay
    }
}

char scancode_to_ascii(uint8_t scancode)
{
    // Scancode to ASCII mapping table
    switch (scancode)
    {
    case 0x02:
        return '1';
    case 0x03:
        return '2';
    case 0x04:
        return '3';
    case 0x05:
        return '4';
    case 0x06:
        return '5';
    case 0x07:
        return '6';
    case 0x08:
        return '7';
    case 0x09:
        return '8';
    case 0x0A:
        return '9';
    case 0x0B:
        return '0';
    case 0x10:
        return 'q';
    case 0x11:
        return 'w';
    case 0x12:
        return 'e';
    case 0x13:
        return 'r';
    case 0x14:
        return 't';
    case 0x15:
        return 'y';
    case 0x16:
        return 'u';
    case 0x17:
        return 'i';
    case 0x18:
        return 'o';
    case 0x19:
        return 'p';
    case 0x1E:
        return 'a';
    case 0x1F:
        return 's';
    case 0x20:
        return 'd';
    case 0x21:
        return 'f';
    case 0x22:
        return 'g';
    case 0x23:
        return 'h';
    case 0x24:
        return 'j';
    case 0x25:
        return 'k';
    case 0x26:
        return 'l';
    case 0x2C:
        return 'z';
    case 0x2D:
        return 'x';
    case 0x2E:
        return 'c';
    case 0x2F:
        return 'v';
    case 0x30:
        return 'b';
    case 0x31:
        return 'n';
    case 0x32:
        return 'm';
    case 0x39:
        return ' '; // Space
    default:
        return 0; // Unknown key
    }
}

void keyboard_clear_buffer(void)
{
    // Aggressively drain keyboard buffer
    for (int clear_attempts = 0; clear_attempts < 100; clear_attempts++)
    {
        uint8_t dummy = inb(KEYBOARD_DATA_PORT);
        (void)dummy;
        for (volatile int j = 0; j < 10000; j++);
    }
    
    // Reset internal state
    kb_state.buffer_pos = 0;
    kb_state.last_scancode = 0;
    kb_state.held_key = 0;
    
    // Clear input buffer
    for (int i = 0; i < 256; i++) {
        kb_state.input_buffer[i] = 0;
    }
}

char *keyboard_get_input(void)
{
    // Clear everything first
    kb_state.buffer_pos = 0;
    for (int i = 0; i < 256; i++) {
        kb_state.input_buffer[i] = 0;
    }
    
    // MASSIVE buffer drain - clear ALL garbage
    for (int i = 0; i < 1000; i++) {
        inb(KEYBOARD_DATA_PORT);
        for (volatile int j = 0; j < 1000; j++);
    }

    // Show cursor
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putchar('_');

    // Simple state machine: wait for actual user input
    int waiting_for_first_key = 1;
    uint32_t consecutive_zeros = 0;

    while (1)
    {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);
        
        // Count consecutive zero reads
        if (scancode == 0) {
            consecutive_zeros++;
            // Only start accepting input after seeing lots of zeros (clean state)
            if (consecutive_zeros > 10000 && waiting_for_first_key) {
                waiting_for_first_key = 0; // Now ready for real input
            }
            for (volatile int i = 0; i < 100; i++);
            continue;
        }
        
        consecutive_zeros = 0;
        
        // Still waiting for clean state? Ignore all scancodes
        if (waiting_for_first_key) {
            continue;
        }

        // Ignore key releases
        if (scancode & 0x80) {
            continue;
        }

        // Handle Enter - return the command
        if (scancode == 0x1C) {
            vga_print("\b\n");
            kb_state.input_buffer[kb_state.buffer_pos] = '\0';
            return kb_state.input_buffer;
        }

        // Handle Backspace
        if (scancode == 0x0E) {
            if (kb_state.buffer_pos > 0) {
                kb_state.buffer_pos--;
                vga_print("\b \b\b");
            }
            vga_putchar('_');
            continue;
        }

        // Handle normal keys
        char key = scancode_to_ascii(scancode);
        if (key != 0 && kb_state.buffer_pos < 255) {
            kb_state.input_buffer[kb_state.buffer_pos++] = key;
            vga_print("\b");
            vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
            vga_putchar(key);
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            vga_putchar('_');
        }
        
        // Delay to prevent double-reads
        for (volatile int i = 0; i < 100000; i++);
    }
}

// Legacy functions for compatibility
char keyboard_getchar(void)
{
    return 0;
}

void keyboard_handler_internal(void)
{
    // Legacy interrupt handler - disabled
}
