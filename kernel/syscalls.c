#include "syscalls.h"
#include "proc/process.h"
#include "kernel.h"

/**
 * System call dispatcher
 */
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    switch (syscall_num) {
        case SYS_EXIT:
            sys_exit((int)arg1);
            return 0;
            
        case SYS_FORK:
            return sys_fork();
            
        case SYS_EXEC:
            return sys_exec((const char*)arg1, (char* const*)arg2);
            
        case SYS_WAIT:
            return sys_wait((uint32_t*)arg1);
            
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
    if (current_process) {
        current_process->state = PROCESS_TERMINATED;
        current_process->exit_code = exit_code;
        
        vga_print("Process ");
        vga_print_hex(current_process->pid);
        vga_print(" exited with code ");
        vga_print_hex(exit_code);
        vga_print("\n");
        
        // Remove from ready queue if present
        if (current_process->state == PROCESS_READY) {
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
    if (!current_process) {
        return -1; // Cannot fork without current process
    }
    
    // Create new process as copy of current
    process_t *child = process_create_copy(current_process);
    if (!child) {
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
    if (!current_process || !program) {
        return -1;
    }
    
    // For now, just simulate exec by printing what would happen
    vga_print("EXEC: Would replace process ");
    vga_print_hex(current_process->pid);
    vga_print(" with program: ");
    vga_print(program);
    vga_print("\n");
    
    // In a real implementation, we would:
    // 1. Load the new program from disk
    // 2. Replace the process memory image
    // 3. Reset the CPU state to start at the new program entry point
    // 4. Update process name and other metadata
    
    return 0; // Success (simulated)
}

/**
 * Wait for child process to terminate
 */
uint32_t sys_wait(uint32_t *status)
{
    if (!current_process) {
        return -1;
    }
    
    // Find terminated child process
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t *proc = process_table[i];
        if (proc && proc->parent_pid == current_process->pid && 
            proc->state == PROCESS_TERMINATED) {
            
            uint32_t child_pid = proc->pid;
            if (status) {
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
    if (pid == 1) {
        vga_print("ERROR: Cannot kill PID 1 (kernel process) - system critical!\n");
        return -1; // Cannot kill kernel process
    }
    
    process_t *target = process_find_by_pid(pid);
    if (!target) {
        return -1; // Process not found
    }
    
    // Additional protection for kernel process
    extern process_t *kernel_process;
    if (target == kernel_process) {
        vga_print("ERROR: Cannot kill kernel_idle process - system critical!\n");
        return -1;
    }
    
    // For now, just simulate signal 9 (SIGKILL)
    if (signal == 9) {
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
