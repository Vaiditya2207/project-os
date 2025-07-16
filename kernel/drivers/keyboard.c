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

    // Initialize modifier states
    kb_state.shift_pressed = 0;
    kb_state.caps_lock_on = 0;
    kb_state.ctrl_pressed = 0;
    kb_state.alt_pressed = 0;

    // Clear input buffer
    for (int i = 0; i < 256; i++)
    {
        kb_state.input_buffer[i] = 0;
    }

    // IMPORTANT: Drain keyboard buffer completely to remove any garbage
    for (int i = 0; i < 100; i++)
    {
        inb(KEYBOARD_DATA_PORT); // Clear any pending data
        for (volatile int j = 0; j < 1000; j++)
            ; // Small delay
    }
}

char scancode_to_ascii(uint8_t scancode)
{
    // Check if caps lock affects this key (letters only)
    uint8_t caps_effect = 0;

    // Handle letters with caps lock and shift
    switch (scancode)
    {
    // Numbers (affected by shift for special characters)
    case 0x02:
        return kb_state.shift_pressed ? '!' : '1';
    case 0x03:
        return kb_state.shift_pressed ? '@' : '2';
    case 0x04:
        return kb_state.shift_pressed ? '#' : '3';
    case 0x05:
        return kb_state.shift_pressed ? '$' : '4';
    case 0x06:
        return kb_state.shift_pressed ? '%' : '5';
    case 0x07:
        return kb_state.shift_pressed ? '^' : '6';
    case 0x08:
        return kb_state.shift_pressed ? '&' : '7';
    case 0x09:
        return kb_state.shift_pressed ? '*' : '8';
    case 0x0A:
        return kb_state.shift_pressed ? '(' : '9';
    case 0x0B:
        return kb_state.shift_pressed ? ')' : '0';

    // Letters (affected by caps lock and shift)
    case 0x10: // Q
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'Q' : 'q';
    case 0x11: // W
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'W' : 'w';
    case 0x12: // E
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'E' : 'e';
    case 0x13: // R
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'R' : 'r';
    case 0x14: // T
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'T' : 't';
    case 0x15: // Y
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'Y' : 'y';
    case 0x16: // U
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'U' : 'u';
    case 0x17: // I
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'I' : 'i';
    case 0x18: // O
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'O' : 'o';
    case 0x19: // P
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'P' : 'p';
    case 0x1E: // A
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'A' : 'a';
    case 0x1F: // S
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'S' : 's';
    case 0x20: // D
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'D' : 'd';
    case 0x21: // F
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'F' : 'f';
    case 0x22: // G
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'G' : 'g';
    case 0x23: // H
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'H' : 'h';
    case 0x24: // J
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'J' : 'j';
    case 0x25: // K
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'K' : 'k';
    case 0x26: // L
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'L' : 'l';
    case 0x2C: // Z
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'Z' : 'z';
    case 0x2D: // X
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'X' : 'x';
    case 0x2E: // C
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'C' : 'c';
    case 0x2F: // V
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'V' : 'v';
    case 0x30: // B
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'B' : 'b';
    case 0x31: // N
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'N' : 'n';
    case 0x32: // M
        caps_effect = (kb_state.caps_lock_on != kb_state.shift_pressed);
        return caps_effect ? 'M' : 'm';

    // Special characters
    case MINUS_SCANCODE:
        return kb_state.shift_pressed ? '_' : '-';
    case EQUALS_SCANCODE:
        return kb_state.shift_pressed ? '+' : '=';
    case LEFT_BRACKET_SCANCODE:
        return kb_state.shift_pressed ? '{' : '[';
    case RIGHT_BRACKET_SCANCODE:
        return kb_state.shift_pressed ? '}' : ']';
    case SEMICOLON_SCANCODE:
        return kb_state.shift_pressed ? ':' : ';';
    case QUOTE_SCANCODE:
        return kb_state.shift_pressed ? '"' : '\'';
    case GRAVE_SCANCODE:
        return kb_state.shift_pressed ? '~' : '`';
    case BACKSLASH_SCANCODE:
        return kb_state.shift_pressed ? '|' : '\\';
    case COMMA_SCANCODE:
        return kb_state.shift_pressed ? '<' : ',';
    case PERIOD_SCANCODE:
        return kb_state.shift_pressed ? '>' : '.';
    case SLASH_SCANCODE:
        return kb_state.shift_pressed ? '?' : '/';
    case SPACE_SCANCODE:
        return ' ';
    case TAB_SCANCODE:
        return '\t';

    default:
        return 0; // Unknown key
    }
}

void keyboard_update_modifiers(uint8_t scancode, uint8_t key_released)
{
    if (key_released)
    {
        // Handle key releases
        switch (scancode)
        {
        case LEFT_SHIFT_SCANCODE:
        case RIGHT_SHIFT_SCANCODE:
            kb_state.shift_pressed = 0;
            break;
        case LEFT_CTRL_SCANCODE:
            kb_state.ctrl_pressed = 0;
            break;
        case LEFT_ALT_SCANCODE:
            kb_state.alt_pressed = 0;
            break;
        }
    }
    else
    {
        // Handle key presses
        switch (scancode)
        {
        case LEFT_SHIFT_SCANCODE:
        case RIGHT_SHIFT_SCANCODE:
            kb_state.shift_pressed = 1;
            break;
        case LEFT_CTRL_SCANCODE:
            kb_state.ctrl_pressed = 1;
            break;
        case LEFT_ALT_SCANCODE:
            kb_state.alt_pressed = 1;
            break;
        case CAPS_LOCK_SCANCODE:
            // Toggle caps lock on key press only
            kb_state.caps_lock_on = !kb_state.caps_lock_on;
            break;
        }
    }
}

void keyboard_clear_buffer(void)
{
    // Drain keyboard buffer
    for (int clear_attempts = 0; clear_attempts < 3; clear_attempts++)
    {
        uint8_t dummy = inb(KEYBOARD_DATA_PORT);
        (void)dummy;
        for (volatile int j = 0; j < BUFFER_CLEAR_DELAY; j++)
            ;
    }
}

char *keyboard_get_input(void)
{
    // Reset buffer position
    kb_state.buffer_pos = 0;

    // Show cursor
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putchar('_');

    while (1)
    {
        // Poll keyboard port for input
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);

        // Reduce Enter cooldown over time
        if (kb_state.enter_cooldown > 0)
        {
            kb_state.enter_cooldown--;
        }

        // Handle key releases (scancode >= 0x80)
        if (scancode >= 0x80)
        {
            uint8_t released_key = scancode - 0x80;

            // Update modifier key states
            keyboard_update_modifiers(released_key, 1);

            if (released_key == kb_state.held_key)
            {
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
        if (scancode > 0 && scancode < 0x80)
        {
            // If this is the same key as before, handle repeat logic
            if (scancode == kb_state.held_key && kb_state.held_key != 0)
            {
                // Key is being held, increment hold time
                kb_state.key_hold_time++;

                if (kb_state.repeat_delay > 0)
                {
                    kb_state.repeat_delay--;
                }
                else if (kb_state.held_key_char != 0)
                {
                    // Time to repeat!
                    vga_print("\b"); // Remove cursor

                    if (kb_state.held_key == BACKSPACE_SCANCODE)
                    {
                        // Repeat backspace
                        if (kb_state.buffer_pos > 0)
                        {
                            kb_state.buffer_pos--;
                            vga_print("\b ");
                            vga_print("\b");
                        }
                    }
                    else if (kb_state.held_key_char != '\b' && kb_state.buffer_pos < 255)
                    {
                        // Repeat normal character
                        kb_state.input_buffer[kb_state.buffer_pos++] = kb_state.held_key_char;
                        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                        vga_putchar(kb_state.held_key_char);
                        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    }

                    vga_putchar('_'); // Show cursor again

                    // Progressive acceleration: faster repeat rate after holding longer
                    if (kb_state.key_hold_time > KEY_REPEAT_ACCELERATION_THRESHOLD)
                    {
                        kb_state.repeat_delay = KEY_REPEAT_FAST_RATE; // Super fast repeat
                    }
                    else
                    {
                        kb_state.repeat_delay = KEY_REPEAT_RATE; // Normal repeat rate
                    }
                }
            }
            else if (scancode != kb_state.last_scancode)
            {
                // New key pressed
                kb_state.last_scancode = scancode;

                // Update modifier key states first
                keyboard_update_modifiers(scancode, 0);

                // Don't repeat modifier keys or caps lock
                if (scancode == LEFT_SHIFT_SCANCODE || scancode == RIGHT_SHIFT_SCANCODE ||
                    scancode == LEFT_CTRL_SCANCODE || scancode == LEFT_ALT_SCANCODE ||
                    scancode == CAPS_LOCK_SCANCODE)
                {
                    vga_putchar('_'); // Just show cursor again
                    continue;
                }

                kb_state.held_key = scancode;
                kb_state.key_hold_time = 0;
                kb_state.repeat_delay = KEY_REPEAT_INITIAL_DELAY;

                // Remove cursor
                vga_print("\b");

                // Handle Enter key with cooldown protection
                if (scancode == ENTER_SCANCODE)
                {
                    if (kb_state.enter_cooldown == 0)
                    {
                        vga_print("\n");
                        kb_state.input_buffer[kb_state.buffer_pos] = '\0';

                        // Clear keyboard buffer
                        keyboard_clear_buffer();

                        // Set cooldown for Enter key
                        kb_state.enter_cooldown = ENTER_COOLDOWN_CYCLES;
                        kb_state.last_scancode = 0;
                        kb_state.held_key = 0; // Don't repeat Enter
                        return kb_state.input_buffer;
                    }
                    else
                    {
                        // Enter is in cooldown, ignore
                        vga_putchar('_');
                        continue;
                    }
                }

                // Handle backspace
                if (scancode == BACKSPACE_SCANCODE)
                {
                    if (kb_state.buffer_pos > 0)
                    {
                        kb_state.buffer_pos--;
                        vga_print("\b ");
                        vga_print("\b");
                    }
                    kb_state.held_key_char = '\b'; // Set for repeat
                    vga_putchar('_');
                    continue;
                }

                // Handle tab key - add 4 spaces for simplicity
                if (scancode == TAB_SCANCODE)
                {
                    for (int i = 0; i < 4 && kb_state.buffer_pos < 252; i++)
                    {
                        kb_state.input_buffer[kb_state.buffer_pos++] = ' ';
                        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                        vga_putchar(' ');
                        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    }
                    kb_state.held_key_char = 0; // Don't repeat tab
                    vga_putchar('_');
                    continue;
                }

                // Handle normal character keys
                char key = scancode_to_ascii(scancode);
                if (key != 0 && kb_state.buffer_pos < 255)
                {
                    kb_state.input_buffer[kb_state.buffer_pos++] = key;
                    kb_state.held_key_char = key; // Store for repeat
                    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                    vga_putchar(key);
                    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                }
                else
                {
                    kb_state.held_key_char = 0; // Don't repeat unknown keys
                }

                // Show cursor again
                vga_putchar('_');
            }
        }
        else if (scancode == 0)
        {
            // No key activity, check if we should continue repeating
            if (kb_state.held_key != 0 && kb_state.held_key_char != 0)
            {
                kb_state.key_hold_time++;

                if (kb_state.repeat_delay > 0)
                {
                    kb_state.repeat_delay--;
                }
                else
                {
                    // Time to repeat!
                    vga_print("\b"); // Remove cursor

                    if (kb_state.held_key == BACKSPACE_SCANCODE)
                    {
                        // Repeat backspace
                        if (kb_state.buffer_pos > 0)
                        {
                            kb_state.buffer_pos--;
                            vga_print("\b ");
                            vga_print("\b");
                        }
                    }
                    else if (kb_state.held_key_char != '\b' && kb_state.buffer_pos < 255)
                    {
                        // Repeat normal character
                        kb_state.input_buffer[kb_state.buffer_pos++] = kb_state.held_key_char;
                        vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                        vga_putchar(kb_state.held_key_char);
                        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                    }

                    vga_putchar('_'); // Show cursor again

                    // Progressive acceleration: faster repeat rate after holding longer
                    if (kb_state.key_hold_time > KEY_REPEAT_ACCELERATION_THRESHOLD)
                    {
                        kb_state.repeat_delay = KEY_REPEAT_FAST_RATE; // Super fast repeat
                    }
                    else
                    {
                        kb_state.repeat_delay = KEY_REPEAT_RATE; // Normal repeat rate
                    }
                }
            }
        }

        // Light polling delay
        for (volatile int i = 0; i < 5000; i++)
            ;
    }

    return kb_state.input_buffer;
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
