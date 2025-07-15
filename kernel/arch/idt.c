/* Interrupt Descriptor Table (IDT) */

#include "../kernel.h"

#define IDT_ENTRIES 256

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt_entries[IDT_ENTRIES];
static struct idt_ptr idt_p;

// External assembly function to load IDT
extern void idt_flush(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}

void idt_init(void) {
    idt_p.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idt_p.base = (uint32_t)&idt_entries;
    
    // Clear IDT entries
    memset(&idt_entries, 0, sizeof(struct idt_entry) * IDT_ENTRIES);
    
    // Remap PIC (Programmable Interrupt Controller)
    // Master PIC
    outb(0x20, 0x11);  // Initialize command
    outb(0x21, 0x20);  // Offset (IRQ 0-7 -> interrupts 32-39)
    outb(0x21, 0x04);  // Tell master PIC there's a slave at IRQ2
    outb(0x21, 0x01);  // 8086 mode
    
    // Slave PIC
    outb(0xA0, 0x11);  // Initialize command
    outb(0xA1, 0x28);  // Offset (IRQ 8-15 -> interrupts 40-47)
    outb(0xA1, 0x02);  // Tell slave PIC its cascade identity
    outb(0xA1, 0x01);  // 8086 mode
    
    // Mask all interrupts except keyboard (IRQ1)
    outb(0x21, 0xFD);  // Master PIC mask (11111101)
    outb(0xA1, 0xFF);  // Slave PIC mask (11111111)
    
    // Load IDT
    idt_flush((uint32_t)&idt_p);
    
    // Enable interrupts
    asm volatile("sti");
}
