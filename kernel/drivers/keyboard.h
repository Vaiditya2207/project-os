#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../kernel.h"

// Keyboard configuration constants
#define KEYBOARD_DATA_PORT 0x60
#define ENTER_SCANCODE 0x1C
#define BACKSPACE_SCANCODE 0x0E
#define ENTER_COOLDOWN_CYCLES 5000  
#define KEY_DELAY_CYCLES 100000      
#define BUFFER_CLEAR_DELAY 100000    

// Keyboard state structure
typedef struct {
    uint8_t last_scancode;
    int enter_cooldown;
    int buffer_pos;
    char input_buffer[256];
} keyboard_state_t;

// Function declarations
char* keyboard_get_input(void);
char scancode_to_ascii(uint8_t scancode);
void keyboard_init(void);
void keyboard_clear_buffer(void);

#endif
