#include "timer.h"
#include "../kernel.h"
#include "../proc/process.h"

// Assembly wrapper for timer interrupt (defined in interrupts.asm)
extern void timer_interrupt_wrapper(void);

static uint32_t timer_ticks = 0;

/**
 * Timer interrupt handler
 */
void timer_handler(void)
{
    timer_ticks++;
    
    // Call scheduler every timer tick for preemptive multitasking
    scheduler_tick();
}

/**
 * Initialize PIT (Programmable Interval Timer)
 */
void timer_init(void)
{
    vga_print("  Initializing timer (PIT)...\n");
    
    // Calculate divisor for desired frequency
    uint32_t divisor = 1193180 / TIMER_FREQUENCY;
    
    // Send command byte to PIT
    outb(0x43, 0x36);  // Channel 0, lobyte/hibyte, rate generator
    
    // Send divisor
    outb(0x40, divisor & 0xFF);        // Low byte
    outb(0x40, (divisor >> 8) & 0xFF); // High byte
    
    // Install timer handler in IDT (IRQ 0 = interrupt 32)
    idt_set_gate(32, (uint32_t)timer_interrupt_wrapper, 0x08, 0x8E);
    
    vga_print("  Timer initialized at ");
    vga_print_hex(TIMER_FREQUENCY);
    vga_print(" Hz\n");
}

/**
 * Get current timer ticks
 */
uint32_t timer_get_ticks(void)
{
    return timer_ticks;
}

/**
 * Sleep for specified number of timer ticks
 */
void timer_sleep(uint32_t ticks)
{
    uint32_t start_ticks = timer_ticks;
    while (timer_ticks < start_ticks + ticks) {
        // Busy wait - in real OS would yield to other processes
        asm volatile("nop");
    }
}
