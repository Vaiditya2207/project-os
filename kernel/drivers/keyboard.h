#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../kernel.h"

// Keyboard configuration constants
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ENTER_SCANCODE 0x1C
#define BACKSPACE_SCANCODE 0x0E
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
} keyboard_state_t;

// Function declarations
char *keyboard_get_input(void);
char scancode_to_ascii(uint8_t scancode);
void keyboard_init(void);
void keyboard_clear_buffer(void);

#endif
