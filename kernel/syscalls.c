#include "syscalls.h"
#include "proc/process.h"
#include "kernel.h"
// Include demo process prototypes
#include "proc/demo_processes.h"
// Function prototypes for missing helpers
// Simple program lookup for simulated exec
void *exec_find_program(const char *program)
{
    vga_print("Trying to find program: ");
    vga_print("\n");
    extern void exec_hello_program(void);
    if (strcmp(program, "hello") == 0)
    {
        return (void *)exec_hello_program;
    } else if (strcmp(program, "counter") == 0)
    {
        return (void *)demo_counter_process;
    } else if (strcmp(program, "calc") == 0)
    {
        return (void *)demo_calc_process;
    } else if (strcmp(program, "monitor") == 0)
    {
        return (void *)demo_monitor_process;
    }

    // Add more programs here as needed
    return NULL;
}
void *exec_find_program(const char *program);
void string_copy(char *dest, const char *src);

/**
 * System call dispatcher
 */
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    switch (syscall_num)
    {
    case SYS_EXIT:
        sys_exit((int)arg1);
        return 0;

    case SYS_FORK:
        return sys_fork();

    case SYS_EXEC:
        return sys_exec((const char *)arg1, (char *const *)arg2);

    case SYS_WAIT:
        return sys_wait((uint32_t *)arg1);

    case SYS_GETPID:
        return sys_getpid();

    case SYS_KILL:
        return sys_kill(arg1, (int)arg2);

    default:
        vga_print("Unknown system call: ");
        vga_print_hex(syscall_num);
        vga_print("\n");
        return -1;
    }
}

/**
 * Exit current process
 */
void sys_exit(int exit_code)
{
    if (current_process)
    {
        current_process->state = PROCESS_TERMINATED;
        current_process->exit_code = exit_code;

        vga_print("Process ");
        vga_print_hex(current_process->pid);
        vga_print(" exited with code ");
        vga_print_hex(exit_code);
        vga_print("\n");

        // Remove from ready queue if present
        if (current_process->state == PROCESS_READY)
        {
            remove_from_ready_queue(current_process);
        }

        // Schedule next process
        current_process = NULL;
        schedule();
    }
}

/**
 * Fork - create copy of current process
 */
uint32_t sys_fork(void)
{
    if (!current_process)
    {
        return -1; // Cannot fork without current process
    }

    // Create new process as copy of current
    process_t *child = process_create_copy(current_process);
    if (!child)
    {
        return -1; // Fork failed
    }

    // Set parent-child relationship
    child->parent_pid = current_process->pid;

    // Add child to ready queue
    add_to_ready_queue(child);

    // Return PID to parent, 0 to child (when child runs)
    // For now, just return child PID since we don't have proper process switching yet
    return child->pid;
}

/**
 * Exec - replace current process with new program
 */
int sys_exec(const char *program, char *const argv[])
{
    if (!current_process || !program)
    {
        vga_print("EXEC: Invalid parameters\n");
        return -1;
    }

    // Simulated exec: reset process struct, set entry point, and call the function
    void *entry_point = exec_find_program(program);
    if (!entry_point)
    {
        vga_print("EXEC: Program not found: ");
        vga_print(program);
        vga_print("\n");
        return -1;
    }

    // Reset CPU state
    current_process->cpu_state.eip = (uint32_t)entry_point;
    current_process->cpu_state.eax = 0;
    current_process->cpu_state.ebx = 0;
    current_process->cpu_state.ecx = 0;
    current_process->cpu_state.edx = 0;
    current_process->cpu_state.esi = 0;
    current_process->cpu_state.edi = 0;
    current_process->cpu_state.esp = current_process->stack_base + current_process->stack_size - 4;
    current_process->cpu_state.ebp = current_process->cpu_state.esp;

    // Update process name
    string_copy(current_process->name, program);

    vga_print("EXEC: Replaced process ");
    vga_print_hex(current_process->pid);
    vga_print(" with program: ");
    vga_print(program);
    vga_print("\n");

    // Simulate context switch by calling the function directly
    void (*program_func)(void) = (void (*)(void))entry_point;
    if (program_func)
    {
        program_func();
    }

    return 0;
}

/**
 * Wait for child process to terminate
 */
uint32_t sys_wait(uint32_t *status)
{
    if (!current_process)
    {
        return -1;
    }

    // Find terminated child process
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        process_t *proc = process_table[i];
        if (proc && proc->parent_pid == current_process->pid &&
            proc->state == PROCESS_TERMINATED)
        {

            uint32_t child_pid = proc->pid;
            if (status)
            {
                *status = proc->exit_code;
            }

            // Clean up terminated child
            process_cleanup(proc);
            return child_pid;
        }
    }

    // No terminated children found - would normally block here
    return 0;
}

/**
 * Get current process ID
 */
uint32_t sys_getpid(void)
{
    return current_process ? current_process->pid : 0;
}

/**
 * Send signal to process
 */
int sys_kill(uint32_t pid, int signal)
{
    // Protect critical system processes
    if (pid == 1)
    {
        vga_print("ERROR: Cannot kill PID 1 (kernel process) - system critical!\n");
        return -1; // Cannot kill kernel process
    }

    process_t *target = process_find_by_pid(pid);
    if (!target)
    {
        return -1; // Process not found
    }

    // Additional protection for kernel process
    extern process_t *kernel_process;
    if (target == kernel_process)
    {
        vga_print("ERROR: Cannot kill kernel_idle process - system critical!\n");
        return -1;
    }

    // For now, just simulate signal 9 (SIGKILL)
    if (signal == 9)
    {
        target->state = PROCESS_TERMINATED;
        target->exit_code = -1;

        vga_print("Killed process ");
        vga_print_hex(pid);
        vga_print("\n");

        return 0;
    }

    vga_print("Signal ");
    vga_print_hex(signal);
    vga_print(" sent to process ");
    vga_print_hex(pid);
    vga_print("\n");

    return 0;
}
