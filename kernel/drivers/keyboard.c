/* Keyboard Driver */

#include "../kernel.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static char keyboard_buffer = 0;

// External assembly wrapper
extern void keyboard_handler_wrapper(void);

// US QWERTY keyboard layout
static char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Keyboard interrupt handler
void keyboard_handler_internal(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Only handle key press events (bit 7 clear)
    if (!(scancode & 0x80)) {
        keyboard_buffer = keyboard_map[scancode];
    }
    
    // Send End of Interrupt signal to PIC
    outb(0x20, 0x20);
}

void keyboard_init(void) {
    // Disabled for v1.1 - will add back later
    keyboard_buffer = 0;
    // Skip interrupt setup for now
}

char keyboard_getchar(void) {
    // Disabled for v1.1 - return nothing for now
    return 0;
}
