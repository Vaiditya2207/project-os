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
 * Create a new process
 */
process_t *process_create(const char *name, void *entry_point, process_priority_t priority)
{
    // Allocate process control block (PCB)
    process_t *process = (process_t *)kmalloc(sizeof(process_t));
    if (!process) {
        return NULL;
    }

    // Initialize all memory to zero first (safer approach)
    for (int i = 0; i < sizeof(process_t); i++) {
        ((char*)process)[i] = 0;
    }

    // Allocate unique PID
    uint32_t pid = process_allocate_pid();
    if (pid == 0) {
        kfree(process);
        return NULL;
    }

    // Allocate stack memory
    void *stack = kmalloc(STACK_SIZE);
    if (!stack) {
        kfree(process);
        return NULL;
    }

    // Initialize basic fields (avoid the problematic state field for now)
    process->pid = pid;
    process->parent_pid = 0;
    process->priority = priority;
    
    // Initialize CPU state
    process->cpu_state.eip = (uint32_t)entry_point;
    process->cpu_state.esp = (uint32_t)stack + STACK_SIZE - 4;
    process->cpu_state.eflags = 0x202;
    
    // Memory management
    process->stack_base = (uint32_t)stack;
    process->stack_size = STACK_SIZE;
    process->heap_base = 0;
    process->heap_size = 0;
    
    // Scheduling information
    process->time_slice = DEFAULT_TIME_SLICE;
    process->total_runtime = 0;
    process->sleep_until = 0;
    
    // Process relationships - already zeroed
    
    // Copy process name
    int i;
    for (i = 0; i < 63 && name[i] != '\0'; i++) {
        process->name[i] = name[i];
    }
    process->name[i] = '\0';

    // Add to process table
    process_table[pid] = process;

    // Set state last (this was the problematic field)
    process->state = PROCESS_READY;
    
    // Add to ready queue
    add_to_ready_queue(process);

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
 * Exit current process
 */
void process_exit(int exit_code)
{
    if (!current_process || current_process == kernel_process)
    {
        return; // Can't exit kernel process
    }

    current_process->state = PROCESS_TERMINATED;

    // TODO: Notify parent process
    // TODO: Clean up child processes

    // Switch to next process
    schedule();
}

/**
 * Allocate a unique process ID
 */
uint32_t process_allocate_pid(void)
{
    // Simple PID allocation - find first free PID
    for (uint32_t pid = next_pid; pid < MAX_PROCESSES; pid++)
    {
        if (process_table[pid] == NULL)
        {
            next_pid = pid + 1;
            return pid;
        }
    }

    // Wrap around and search from beginning
    for (uint32_t pid = 1; pid < next_pid; pid++)
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
    {
        return NULL;
    }
    return process_table[pid];
}

/**
 * Set process state
 */
void process_set_state(process_t *process, process_state_t state)
{
    if (!process)
        return;

    process_state_t old_state = process->state;
    process->state = state;

    // Handle state transitions
    if (old_state == PROCESS_READY && state != PROCESS_READY)
    {
        remove_from_ready_queue(process);
    }
    else if (old_state != PROCESS_READY && state == PROCESS_READY)
    {
        add_to_ready_queue(process);
    }
}

/**
 * Print process information
 */
void process_print_info(process_t *process)
{
    if (!process)
        return;

    vga_print("Process Info:\n");
    vga_print("  PID: ");
    // TODO: Implement number to string conversion
    vga_print("  Name: ");
    vga_print(process->name);
    vga_print("\n  State: ");

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
}

/**
 * List all processes
 */
void process_list_all(void)
{
    vga_print("Process List:\n");
    vga_print("PID\tNAME\t\tSTATE\n");
    vga_print("---\t----\t\t-----\n");

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i])
        {
            // TODO: Implement better formatting when we have printf
            vga_print(process_table[i]->name);
            vga_print("\n");
        }
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
 * Simple round-robin scheduler
 */
void schedule(void)
{
    if (!ready_queue_head)
    {
        // No processes to run, stay with current (should be kernel idle)
        if (!current_process)
        {
            current_process = kernel_process;
        }
        return;
    }

    process_t *next_process = ready_queue_head;

    // If current process is still ready, put it at end of queue
    if (current_process && current_process->state == PROCESS_READY)
    {
        // Current process goes to back of queue
        remove_from_ready_queue(current_process);
        add_to_ready_queue(current_process);
    }

    // Switch to next process
    if (next_process != current_process)
    {
        process_t *old_process = current_process;
        current_process = next_process;
        current_process->state = PROCESS_RUNNING;

        // Remove from ready queue since it's now running
        remove_from_ready_queue(current_process);

        // Perform context switch (placeholder for now)
        // context_switch(old_process, current_process);
    }
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

// Simple test function with same signature
process_t *process_create_test(const char *name, void *entry_point, process_priority_t priority)
{
    vga_print("TEST: Simple function called successfully\n");
    return NULL; // Just return NULL for test
}
