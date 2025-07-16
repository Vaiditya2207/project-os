#ifndef PROCESS_H
#define PROCESS_H

#include "kernel.h"

// Process states
typedef enum
{
    PROCESS_READY = 0,     // Ready to run
    PROCESS_RUNNING = 1,   // Currently executing
    PROCESS_BLOCKED = 2,   // Waiting for I/O or resource
    PROCESS_TERMINATED = 3 // Finished execution
} process_state_t;

// Process priority levels
typedef enum
{
    PRIORITY_HIGH = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_LOW = 2
} process_priority_t;

// CPU register state for context switching
typedef struct
{
    // General purpose registers
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;

    // Segment registers
    uint32_t eip;                    // Instruction pointer
    uint32_t eflags;                 // CPU flags
    uint32_t cs, ds, es, fs, gs, ss; // Segment selectors
} cpu_state_t;

// Process Control Block (PCB)
typedef struct process
{
    uint32_t pid;        // Process ID
    uint32_t parent_pid; // Parent process ID
    char name[64];       // Process name

    process_state_t state;       // Current process state
    process_priority_t priority; // Process priority
    int exit_code;               // Exit code when terminated

    cpu_state_t cpu_state;    // Saved CPU state
    uint32_t *page_directory; // Virtual memory page directory

    // Memory management
    uint32_t stack_base; // Stack base address
    uint32_t stack_size; // Stack size
    uint32_t heap_base;  // Heap base address
    uint32_t heap_size;  // Current heap size
    uint32_t memory_used; // Total memory used

    // Scheduling information
    uint32_t time_slice;    // Time quantum for scheduling
    uint32_t total_runtime; // Total CPU time used
    uint32_t sleep_until;   // Wake up time (if sleeping)

    // File descriptors (for future file system)
    void *file_descriptors[16]; // File descriptor table

    // Process relationships
    struct process *parent;     // Pointer to parent process
    struct process *children;   // Linked list of child processes
    struct process *next_child; // Next sibling in children list

    // Scheduler queue pointers
    struct process *next; // Next process in scheduler queue
    struct process *prev; // Previous process in scheduler queue
} process_t;

//Process manager configuration
#define MAX_PROCESSES 256
#define DEFAULT_TIME_SLICE 10 // Default time slice in timer ticks
#define STACK_SIZE 4096       // Default stack size (4KB)

// Process operation return codes
#define PROCESS_SUCCESS 1
#define PROCESS_NOT_FOUND 0
#define PROCESS_PROTECTED -1
#define PROCESS_ERROR -2

// Global process table
extern process_t *process_table[MAX_PROCESSES];

// Process management functions
void process_init(void);
process_t *process_create(const char *name, void *entry_point, process_priority_t priority);
void process_destroy(process_t *process);
void process_exit(int exit_code);

// Scheduler functions
void scheduler_init(void);
void schedule(void);
process_t *scheduler_get_current(void);
void scheduler_add_process(process_t *process);
void scheduler_remove_process(process_t *process);

// Context switching
void context_switch(process_t *old_process, process_t *new_process);
void save_cpu_state(cpu_state_t *state);
void restore_cpu_state(cpu_state_t *state);

// Process state management
void process_set_state(process_t *process, process_state_t state);
void process_sleep(process_t *process, uint32_t ticks);
void process_wake(process_t *process);

// Process utilities
process_t *process_find_by_pid(uint32_t pid);
uint32_t process_allocate_pid(void);
void process_print_info(process_t *process);
void process_list_all(void);
process_t *process_find_by_pid(uint32_t pid);
int process_kill_by_pid(uint32_t pid);
int process_set_status(uint32_t pid, process_state_t status);

// Process management functions
process_t *process_create_copy(process_t *parent);
void process_cleanup(process_t *process);
void enable_process_execution(void);
void remove_from_ready_queue(process_t *process);
void add_to_ready_queue(process_t *process);

// Context switching functions (implemented in assembly)
void save_context(process_t *process);
void restore_context(process_t *process);
void context_switch(process_t *old_process, process_t *new_process);
void switch_to_process(process_t *new_process);

// Scheduler functions
void scheduler_tick(void);
process_t *scheduler_get_next(void);

// Kernel process (idle task)
extern process_t *kernel_process;
extern process_t *current_process;

// Demo processes for testing
void create_demo_processes(void);
void demo_counter_process(void);
void demo_calc_process(void);
void demo_monitor_process(void);

#endif
