#include "process.h"

// Global process management state
static process_t *process_table[MAX_PROCESSES];
static uint32_t next_pid = 1;
static process_t *ready_queue_head = NULL;
static process_t *ready_queue_tail = NULL;

// Current running process and kernel idle process
process_t *current_process = NULL;
process_t *kernel_process = NULL;

// Forward declarations
static void process_idle_task(void);
static void add_to_ready_queue(process_t *process);
static void remove_from_ready_queue(process_t *process);
process_t *process_create_test(const char *name, void *entry_point, process_priority_t priority);

/**
 * Initialize the process management subsystem
 */
void process_init(void)
{
    vga_print("  Initializing process table...\n");

    // Initialize process table
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i] = NULL;
    }

    vga_print("  Process table initialized...\n");

    // Initialize global state
    current_process = NULL;
    kernel_process = NULL;
    next_pid = 1;
    ready_queue_head = NULL;
    ready_queue_tail = NULL;

    vga_print("  Process management ready\n");
}

/**
 * Create a new process - SAFE VERSION WITHOUT EXECUTION
 */
process_t *process_create(const char *name, void *entry_point, process_priority_t priority)
{
    vga_print("DEBUG: process_create called\n");
    
    // Validate inputs first
    if (!name) {
        vga_print("DEBUG: Invalid name parameter\n");
        return NULL;
    }
    
    vga_print("DEBUG: Checking process count\n");
    
    // Use hybrid approach: static PCBs + dynamic memory for stacks
    static process_t static_processes[10]; 
    static int process_count = 0;
    
    if (process_count >= 10) {
        vga_print("DEBUG: Too many processes\n");
        return NULL; // Too many processes
    }
    
    vga_print("DEBUG: Getting process slot\n");
    process_t *process = &static_processes[process_count];
    
    vga_print("DEBUG: Allocating stack\n");
    // Allocate real stack memory
    void *stack = kmalloc(STACK_SIZE);
    if (!stack) {
        vga_print("DEBUG: Stack allocation failed\n");
        return NULL; // Out of memory for stack
    }
    
    vga_print("DEBUG: Setting basic fields\n");
    // Initialize process with real memory management
    process->pid = process_count + 1;
    process->parent_pid = 0;
    process->priority = priority;
    process->state = PROCESS_READY;
    
    vga_print("DEBUG: Copying name\n");
    // Copy name safely with strict bounds checking to prevent infinite loops
    int i;
    for (i = 0; i < 63 && name && name[i] != '\0'; i++)
    {
        process->name[i] = name[i];
    }
    vga_print("DEBUG: Name copied\n");
    
    vga_print("DEBUG: Setting up CPU state\n");
    // Initialize CPU state with real stack BUT DON'T EXECUTE
    process->cpu_state.eip = (uint32_t)entry_point;
    process->cpu_state.esp = (uint32_t)stack + STACK_SIZE - 4; // Stack grows down
    process->cpu_state.eflags = 0x202; // Enable interrupts flag
    process->cpu_state.cs = 0x08; // Kernel code segment
    process->cpu_state.ds = 0x10; // Kernel data segment
    process->cpu_state.es = 0x10;
    process->cpu_state.fs = 0x10;
    process->cpu_state.gs = 0x10;
    process->cpu_state.ss = 0x10; // Kernel stack segment
    
    vga_print("DEBUG: Setting up memory management\n");
    // Set up memory management
    process->stack_base = (uint32_t)stack;
    process->stack_size = STACK_SIZE;
    process->heap_base = 0; // No heap allocated initially
    process->heap_size = 0;
    
    vga_print("DEBUG: Setting up scheduling info\n");
    // Initialize scheduling info
    process->time_slice = DEFAULT_TIME_SLICE;
    process->total_runtime = 0;
    process->sleep_until = 0;
    
    vga_print("DEBUG: Clearing pointers\n");
    // Clear pointers
    process->next = NULL;
    process->prev = NULL;
    process->parent = NULL;
    process->children = NULL;
    process->next_child = NULL;
    
    vga_print("DEBUG: Clearing file descriptors\n");
    // Clear file descriptors
    for (int j = 0; j < 16; j++) {
        process->file_descriptors[j] = NULL;
    }
    
    vga_print("DEBUG: Adding to process table\n");
    // Add to process table using PID as index
    if (process->pid < MAX_PROCESSES) {
        process_table[process->pid] = process;
    }
    
    vga_print("DEBUG: Incrementing process count\n");
    process_count++;
    
    vga_print("DEBUG: process_create completed successfully\n");
    
    // IMPORTANT: Do NOT execute the entry_point function!
    // Just store it for potential future execution
    // The process exists as a data structure only
    
    return process;
}

/**
 * Destroy a process and free its resources
 */
void process_destroy(process_t *process)
{
    if (!process)
        return;

    // Remove from ready queue
    remove_from_ready_queue(process);

    // Free stack memory
    if (process->stack_base)
    {
        kfree((void *)process->stack_base);
    }

    // Free heap memory (if allocated)
    if (process->heap_base)
    {
        kfree((void *)process->heap_base);
    }

    // Remove from process table
    if (process->pid < MAX_PROCESSES)
    {
        process_table[process->pid] = NULL;
    }

    // Free PCB
    kfree(process);
}

/**
 * Exit current process - DISABLED FOR STABILITY  
 */
void process_exit(int exit_code)
{
    // Process exit disabled to prevent crashes
    // Just mark as terminated but don't actually exit
    if (current_process && current_process != kernel_process)
    {
        current_process->state = PROCESS_TERMINATED;
    }
    
    // Don't call schedule() to avoid crashes
    return;
}

/**
 * Allocate a unique process ID
 */
uint32_t process_allocate_pid(void)
{
    // Check current position first
    if (next_pid < MAX_PROCESSES && process_table[next_pid] == NULL)
    {
        uint32_t pid = next_pid;
        next_pid++;
        return pid;
    }

    // Simple PID allocation - find first free PID
    for (uint32_t pid = 1; pid < MAX_PROCESSES; pid++)
    {
        if (process_table[pid] == NULL)
        {
            next_pid = pid + 1;
            return pid;
        }
    }

    return 0; // No free PIDs
}

/**
 * Find process by PID
 */
process_t *process_find_by_pid(uint32_t pid)
{
    if (pid >= MAX_PROCESSES)
        return NULL;
    return process_table[pid];
}

/**
 * Kill process by PID - FUNCTIONAL VERSION WITH MEMORY CLEANUP
 */
int process_kill_by_pid(uint32_t pid)
{
    process_t *process = process_find_by_pid(pid);
    if (!process)
        return 0; // Process not found

    // Clean up allocated memory
    if (process->stack_base) {
        kfree((void *)process->stack_base);
        process->stack_base = 0;
        process->stack_size = 0;
    }
    
    if (process->heap_base) {
        kfree((void *)process->heap_base);
        process->heap_base = 0;
        process->heap_size = 0;
    }

    // Mark as terminated
    process->state = PROCESS_TERMINATED;

    // Remove from process table
    if (pid < MAX_PROCESSES) {
        process_table[pid] = NULL;
    }

    return 1; // Success
}

/**
 * Set process status - SAFE VERSION
 */
int process_set_status(uint32_t pid, process_state_t status)
{
    process_t *process = process_find_by_pid(pid);
    if (!process)
        return 0; // Process not found

    // Only allow specific states
    if (status != PROCESS_READY && status != PROCESS_BLOCKED && status != PROCESS_TERMINATED)
    {
        return 0; // Invalid status
    }

    // Just update the status field - no queue management
    process->state = status;

    return 1; // Success
}

/**
 * Print process information - FUNCTIONAL VERSION
 */
void process_print_info(process_t *process)
{
    if (!process)
        return;

    vga_print("Process Info:\n");
    vga_print("  PID: ");
    if (process->pid < 10)
    {
        vga_putchar('0' + process->pid);
    }
    else
    {
        vga_putchar('0' + (process->pid / 10));
        vga_putchar('0' + (process->pid % 10));
    }
    vga_print("\n");
    
    vga_print("  Name: ");
    vga_print(process->name);
    vga_print("\n");
    
    vga_print("  State: ");
    switch (process->state)
    {
    case PROCESS_READY:
        vga_print("READY");
        break;
    case PROCESS_RUNNING:
        vga_print("RUNNING");
        break;
    case PROCESS_BLOCKED:
        vga_print("BLOCKED");
        break;
    case PROCESS_TERMINATED:
        vga_print("TERMINATED");
        break;
    }
    vga_print("\n");
    
    // Show memory information
    vga_print("  Stack: ");
    if (process->stack_base) {
        vga_print("0x");
        // Simple hex display for stack base
        uint32_t addr = process->stack_base;
        for (int i = 28; i >= 0; i -= 4) {
            uint32_t digit = (addr >> i) & 0xF;
            if (digit < 10) {
                vga_putchar('0' + digit);
            } else {
                vga_putchar('A' + digit - 10);
            }
        }
        vga_print(" (");
        // Show stack size in KB
        uint32_t kb = process->stack_size / 1024;
        if (kb < 10) {
            vga_putchar('0' + kb);
        } else {
            vga_putchar('0' + (kb / 10));
            vga_putchar('0' + (kb % 10));
        }
        vga_print("KB)");
    } else {
        vga_print("None");
    }
    vga_print("\n");
    
    vga_print("  Entry Point: 0x");
    uint32_t eip = process->cpu_state.eip;
    for (int i = 28; i >= 0; i -= 4) {
        uint32_t digit = (eip >> i) & 0xF;
        if (digit < 10) {
            vga_putchar('0' + digit);
        } else {
            vga_putchar('A' + digit - 10);
        }
    }
    vga_print("\n");
}

/**
 * List all processes - FUNCTIONAL VERSION WITH MEMORY INFO
 */
void process_list_all(void)
{
    vga_print("Process List:\n");
    vga_print("PID\tNAME\t\tSTATE\t\tMEMORY\n");
    vga_print("---\t----\t\t-----\t\t------\n");

    int found_processes = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i])
        {
            found_processes++;
            process_t *proc = process_table[i];

            // Print PID
            if (proc->pid < 10)
            {
                vga_putchar('0' + proc->pid);
            }
            else
            {
                vga_putchar('0' + (proc->pid / 10));
                vga_putchar('0' + (proc->pid % 10));
            }
            vga_print("\t");

            // Print name (truncate if too long)
            int name_len = 0;
            while (proc->name[name_len] && name_len < 12) {
                vga_putchar(proc->name[name_len]);
                name_len++;
            }
            
            // Pad name to align columns
            while (name_len < 12) {
                vga_putchar(' ');
                name_len++;
            }
            vga_print("\t");

            // Print state
            switch (proc->state)
            {
            case PROCESS_READY:
                vga_print("READY\t\t");
                break;
            case PROCESS_RUNNING:
                vga_print("RUNNING\t\t");
                break;
            case PROCESS_BLOCKED:
                vga_print("BLOCKED\t\t");
                break;
            case PROCESS_TERMINATED:
                vga_print("TERMINATED\t");
                break;
            default:
                vga_print("UNKNOWN\t\t");
                break;
            }

            // Print memory usage
            if (proc->stack_base) {
                uint32_t kb = proc->stack_size / 1024;
                if (kb < 10) {
                    vga_putchar('0' + kb);
                } else {
                    vga_putchar('0' + (kb / 10));
                    vga_putchar('0' + (kb % 10));
                }
                vga_print("KB");
            } else {
                vga_print("0KB");
            }
            
            vga_print("\n");
        }
    }

    if (found_processes == 0)
    {
        vga_print("(No processes)\n");
    }
}

/**
 * Add process to ready queue
 */
static void add_to_ready_queue(process_t *process)
{
    if (!process || process->state != PROCESS_READY)
        return;

    process->next = NULL;
    process->prev = ready_queue_tail;

    if (ready_queue_tail)
    {
        ready_queue_tail->next = process;
    }
    else
    {
        ready_queue_head = process;
    }

    ready_queue_tail = process;
}

/**
 * Remove process from ready queue
 */
static void remove_from_ready_queue(process_t *process)
{
    if (!process)
        return;

    if (process->prev)
    {
        process->prev->next = process->next;
    }
    else
    {
        ready_queue_head = process->next;
    }

    if (process->next)
    {
        process->next->prev = process->prev;
    }
    else
    {
        ready_queue_tail = process->prev;
    }

    process->next = NULL;
    process->prev = NULL;
}

/**
 * Simple round-robin scheduler - DISABLED FOR STABILITY
 */
void schedule(void)
{
    // Scheduler disabled for now - just maintain current process
    if (!current_process)
    {
        current_process = kernel_process;
    }
    // Don't actually switch processes to avoid crashes
    return;
}

/**
 * Get current running process
 */
process_t *scheduler_get_current(void)
{
    return current_process;
}

/**
 * Kernel idle task - runs when no other processes are ready
 */
static void process_idle_task(void)
{
    while (1)
    {
        // Idle loop - just busy wait instead of halt for now
        // asm volatile("hlt");  // Don't halt since we don't have timer interrupts
        for (volatile int i = 0; i < 1000000; i++)
        {
            // Busy wait to prevent kernel from exiting
        }
    }
}

/**
 * Initialize scheduler
 */
void scheduler_init(void)
{
    vga_print("  Initializing scheduler...\n");
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    vga_print("  Scheduler ready\n");
}

// Simple test function with same signature - SAFE VERSION WITHOUT KMALLOC
process_t *process_create_test(const char *name, void *entry_point, process_priority_t priority)
{
    vga_print("Creating process without kmalloc...\n");
    
    // Use hybrid approach: static PCBs + static stack arrays (no kmalloc)
    static process_t static_processes[10]; 
    static char static_stacks[10][4096]; // 4KB stacks for each process
    static int process_count = 0;
    
    if (process_count >= 10) {
        vga_print("ERROR: Too many processes\n");
        return NULL; // Too many processes
    }
    
    process_t *process = &static_processes[process_count];
    char *stack = static_stacks[process_count];
    
    vga_print("Initializing process data...\n");
    
    // Initialize process with static memory management
    process->pid = process_count + 1;
    process->parent_pid = 0;
    process->priority = priority;
    process->state = PROCESS_READY;
    
    // Copy name safely
    int i;
    for (i = 0; i < 63 && name && name[i] != '\0'; i++)
    {
        process->name[i] = name[i];
    }
    process->name[i] = '\0';
    
    vga_print("Setting up CPU state...\n");
    
    // Initialize CPU state with static stack
    process->cpu_state.eip = (uint32_t)entry_point;
    process->cpu_state.esp = (uint32_t)stack + 4096 - 4; // Stack grows down
    process->cpu_state.eflags = 0x202; // Enable interrupts flag
    process->cpu_state.cs = 0x08; // Kernel code segment
    process->cpu_state.ds = 0x10; // Kernel data segment
    process->cpu_state.es = 0x10;
    process->cpu_state.fs = 0x10;
    process->cpu_state.gs = 0x10;
    process->cpu_state.ss = 0x10; // Kernel stack segment
    
    // Set up memory management
    process->stack_base = (uint32_t)stack;
    process->stack_size = 4096;
    process->heap_base = 0; // No heap allocated initially
    process->heap_size = 0;
    
    // Initialize scheduling info
    process->time_slice = DEFAULT_TIME_SLICE;
    process->total_runtime = 0;
    process->sleep_until = 0;
    
    // Clear pointers
    process->next = NULL;
    process->prev = NULL;
    process->parent = NULL;
    process->children = NULL;
    process->next_child = NULL;
    
    // Clear file descriptors
    for (int j = 0; j < 16; j++) {
        process->file_descriptors[j] = NULL;
    }
    
    // Add to process table using PID as index
    if (process->pid < MAX_PROCESSES) {
        process_table[process->pid] = process;
    }
    
    process_count++;
    
    vga_print("Process created successfully with static memory!\n");
    
    return process;
}
