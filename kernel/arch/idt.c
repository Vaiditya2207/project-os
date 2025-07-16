/* Interrupt Descriptor Table (IDT) - Fixed Version */

#include "../kernel.h"

#define IDT_ENTRIES 256

struct idt_entry
{
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt_entries[IDT_ENTRIES];
static struct idt_ptr idt_p;

// External assembly function to load IDT
extern void idt_flush(uint32_t);
extern void int3_handler_asm(void);

// Exception handlers
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}

// Default exception handler
void exception_handler(void)
{
    // For now, just hang - don't crash
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_print("\nEXCEPTION OCCURRED - System Halted\n");
    while (1)
    {
        asm volatile("hlt");
    }
}

// Safe INT 3 handler for testing
void int3_handler(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("INT 3 handler called successfully!\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    // This will return normally via iret in the assembly wrapper
}

void idt_init(void)
{
    idt_p.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idt_p.base = (uint32_t)&idt_entries;

    // Clear IDT entries
    for (int i = 0; i < IDT_ENTRIES; i++)
    {
        idt_entries[i].base_low = 0;
        idt_entries[i].sel = 0;
        idt_entries[i].always0 = 0;
        idt_entries[i].flags = 0;
        idt_entries[i].base_high = 0;
    }

    // Set up basic exception handlers first
    // NOTE: Using actual segment values from idtcheck: CS=0x10, DS=0x18
    // TODO: Fix GDT setup to use standard values (CS=0x08, DS=0x10)
    idt_set_gate(0, (uint32_t)exception_handler, 0x10, 0x8E); // Division by zero
    idt_set_gate(1, (uint32_t)exception_handler, 0x10, 0x8E); // Debug
    idt_set_gate(2, (uint32_t)exception_handler, 0x10, 0x8E); // NMI
    idt_set_gate(3, (uint32_t)int3_handler_asm, 0x10, 0x8E); // Breakpoint - special handler
    idt_set_gate(4, (uint32_t)exception_handler, 0x10, 0x8E); // Overflow
    idt_set_gate(5, (uint32_t)exception_handler, 0x10, 0x8E); // Bound range
    idt_set_gate(6, (uint32_t)exception_handler, 0x10, 0x8E); // Invalid opcode
    idt_set_gate(7, (uint32_t)exception_handler, 0x10, 0x8E); // Device not available

    // Load IDT
    idt_flush((uint32_t)&idt_p);
}
