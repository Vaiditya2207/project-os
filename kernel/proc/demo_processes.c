#include "process.h"

// Demo user processes for testing process management

/**
 * Demo process that counts numbers
 */
void demo_counter_process(void)
{
    // This would normally have its own memory space
    // For now, just a simple counting demonstration
    for (int i = 0; i < 1000000; i++)
    {
        // Simulate some work
        if (i % 100000 == 0)
        {
            // Would print status if we had per-process output
        }
    }

    // Process exits naturally
    process_exit(0);
}

/**
 * Demo process that performs calculations
 */
void demo_calc_process(void)
{
    // Simulate mathematical calculations
    int result = 0;
    for (int i = 0; i < 500000; i++)
    {
        result += i * 2;
        if (result > 1000000)
        {
            result = result % 1000000;
        }
    }

    // Process exits naturally
    process_exit(0);
}

/**
 * Demo background process that "monitors" system
 */
void demo_monitor_process(void)
{
    // Simulate a monitoring daemon
    for (int i = 0; i < 2000000; i++)
    {
        // Simulate monitoring work
        if (i % 200000 == 0)
        {
            // Would check system status
        }
    }

    process_exit(0);
}

/**
 * Create demo processes for testing scheduler
 */
void create_demo_processes(void)
{
    // Create several demo processes to test scheduling
    process_create("counter", (void *)demo_counter_process, PRIORITY_NORMAL);
    process_create("calculator", (void *)demo_calc_process, PRIORITY_HIGH);
    process_create("monitor", (void *)demo_monitor_process, PRIORITY_LOW);
}

/**
 * Stress test - create multiple processes
 */
void process_stress_test(void)
{
    vga_print("Creating stress test processes...\n");

    for (int i = 0; i < 5; i++)
    {
        char name[32];
        // Simple name generation
        name[0] = 't';
        name[1] = 'e';
        name[2] = 's';
        name[3] = 't';
        name[4] = '0' + i;
        name[5] = '\0';

        process_create(name, (void *)demo_counter_process, PRIORITY_NORMAL);
    }

    vga_print("Stress test processes created!\n");
}
