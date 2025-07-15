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
    // Install keyboard interrupt handler (IRQ1 = interrupt 33)
    idt_set_gate(33, (uint32_t)keyboard_handler_wrapper, 0x08, 0x8E);
    
    // Enable keyboard IRQ (IRQ1)
    uint8_t mask = inb(0x21);
    mask &= ~(1 << 1);  // Clear bit 1
    outb(0x21, mask);
    
    keyboard_buffer = 0;
}

char keyboard_getchar(void) {
    char c = keyboard_buffer;
    if (c) {
        keyboard_buffer = 0;  // Clear buffer
    }
    return c;
}
