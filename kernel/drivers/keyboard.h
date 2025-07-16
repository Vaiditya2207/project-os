#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../kernel.h"

// Keyboard configuration constants
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Scancode definitions
#define ENTER_SCANCODE 0x1C
#define BACKSPACE_SCANCODE 0x0E
#define TAB_SCANCODE 0x0F
#define CAPS_LOCK_SCANCODE 0x3A
#define LEFT_SHIFT_SCANCODE 0x2A
#define RIGHT_SHIFT_SCANCODE 0x36
#define LEFT_CTRL_SCANCODE 0x1D
#define LEFT_ALT_SCANCODE 0x38
#define SPACE_SCANCODE 0x39

// Special character scancodes
#define MINUS_SCANCODE 0x0C         // - and _
#define EQUALS_SCANCODE 0x0D        // = and +
#define LEFT_BRACKET_SCANCODE 0x1A  // [ and {
#define RIGHT_BRACKET_SCANCODE 0x1B // ] and }
#define SEMICOLON_SCANCODE 0x27     // ; and :
#define QUOTE_SCANCODE 0x28         // ' and "
#define GRAVE_SCANCODE 0x29         // ` and ~
#define BACKSLASH_SCANCODE 0x2B     // \ and |
#define COMMA_SCANCODE 0x33         // , and <
#define PERIOD_SCANCODE 0x34        // . and >
#define SLASH_SCANCODE 0x35         // / and ?

// Timing constants
#define ENTER_COOLDOWN_CYCLES 5000
#define KEY_DELAY_CYCLES 100000
#define BUFFER_CLEAR_DELAY 100000

// Key repeat configuration
#define KEY_REPEAT_INITIAL_DELAY 10000          // Much shorter initial delay
#define KEY_REPEAT_RATE 5000                    // Base repeat rate
#define KEY_REPEAT_FAST_RATE 5000               // Fast repeat rate after acceleration
#define KEY_REPEAT_ACCELERATION_THRESHOLD 20000 // When to start accelerating

// Keyboard state structure
typedef struct
{
    uint8_t last_scancode;
    int enter_cooldown;
    int buffer_pos;
    char input_buffer[256];
    // Key repeat state
    uint8_t held_key;
    int key_hold_time;
    int repeat_delay;
    char held_key_char;
    // Modifier key states
    uint8_t shift_pressed;
    uint8_t caps_lock_on;
    uint8_t ctrl_pressed;
    uint8_t alt_pressed;
} keyboard_state_t;

// Function declarations
char *keyboard_get_input(void);
char scancode_to_ascii(uint8_t scancode);
char get_special_char(uint8_t scancode, uint8_t shift_pressed);
void keyboard_init(void);
void keyboard_clear_buffer(void);
void keyboard_update_modifiers(uint8_t scancode, uint8_t key_released);

#endif
