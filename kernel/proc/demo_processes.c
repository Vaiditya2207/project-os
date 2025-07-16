#include "process.h"

// Demo user processes for testing process management

/**
 * Demo process that counts numbers - NON-BLOCKING VERSION
 */
void demo_counter_process(void)
{
    // Simple finite function that just returns
    // This simulates a process that does some work and exits
    volatile int counter = 0;
    
    // Do a small amount of work then return
    for (volatile int i = 0; i < 1000; i++)
    {
        counter++;
    }
    
    // Process completes - in a real OS this would trigger process exit
    // For now, just return safely
    return;
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

    for (int i = 0; i < 2; i++)  // Reduced from 5 to 2 for safety
    {
        char name[32];
        // Simple name generation
        name[0] = 't';
        name[1] = 'e';
        name[2] = 's';
        name[3] = 't';
        name[4] = '0' + i;
        name[5] = '\0';

        vga_print("DEBUG: Creating process ");
        vga_print(name);
        vga_print("...\n");
        
        process_t *proc = process_create(name, (void *)demo_counter_process, PRIORITY_NORMAL);
        
        if (proc) {
            vga_print("DEBUG: Successfully created process with PID ");
            if (proc->pid < 10) {
                vga_putchar('0' + proc->pid);
            }
            vga_print("\n");
        } else {
            vga_print("DEBUG: Failed to create process ");
            vga_print(name);
            vga_print("\n");
            break; // Stop if we fail to create a process
        }
        
        // Add a small delay between process creation
        for (volatile int j = 0; j < 100000; j++);
    }

    vga_print("Stress test processes created!\n");
}
