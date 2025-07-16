#include "syscalls.h"
#include "proc/process.h"
#include "kernel.h"

// Global errno for system call error reporting
int32_t errno = 0;

// System call dispatch table
static syscall_handler_t syscall_table[MAX_SYSCALLS];

// Wrapper functions for legacy syscalls to match new interface
int32_t sys_exit_wrapper(struct syscall_context *ctx) {
    sys_exit((int)ctx->ebx);
    return 0;
}

int32_t sys_fork_wrapper(struct syscall_context *ctx) {
    return (int32_t)sys_fork();
}

int32_t sys_exec_wrapper(struct syscall_context *ctx) {
    return (int32_t)sys_exec((const char *)ctx->ebx, (char *const *)ctx->ecx);
}

int32_t sys_wait_wrapper(struct syscall_context *ctx) {
    return (int32_t)sys_wait((uint32_t *)ctx->ebx);
}

int32_t sys_getpid_wrapper(struct syscall_context *ctx) {
    return (int32_t)sys_getpid();
}

int32_t sys_kill_wrapper(struct syscall_context *ctx) {
    return (int32_t)sys_kill(ctx->ebx, (int)ctx->ecx);
}

/**
 * Initialize system call interface
 */
void syscall_init(void)
{
    // Clear dispatch table
    for (int i = 0; i < MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }
    
    // Register system calls with proper wrappers
    syscall_register(SYS_EXIT, sys_exit_wrapper);
    syscall_register(SYS_FORK, sys_fork_wrapper);
    syscall_register(SYS_EXEC, sys_exec_wrapper);
    syscall_register(SYS_WAIT, sys_wait_wrapper);
    syscall_register(SYS_GETPID, sys_getpid_wrapper);
    syscall_register(SYS_KILL, sys_kill_wrapper);
    syscall_register(SYS_READ, sys_read);
    syscall_register(SYS_WRITE, sys_write);
    syscall_register(SYS_OPEN, sys_open);
    syscall_register(SYS_CLOSE, sys_close);
    syscall_register(SYS_YIELD, sys_yield);
    syscall_register(SYS_SLEEP, sys_sleep);
    syscall_register(SYS_BRK, sys_brk);
    syscall_register(SYS_MMAP, sys_mmap);
    syscall_register(SYS_MUNMAP, sys_munmap);
    syscall_register(SYS_MALLOC, sys_malloc_syscall);
    
    // Install INT 0x80 handler in IDT (TEMPORARILY DISABLED FOR DEBUGGING)
    // idt_set_gate(0x80, (uint32_t)syscall_interrupt_handler_debug, 0x08, 0xEE);
    // TODO: Debug why INT 0x80 handler causes immediate crash
    
    vga_print("System call interface initialized (INT 0x80 enabled)\n");
}

/**
 * Register a system call handler
 */
void syscall_register(uint32_t syscall_num, syscall_handler_t handler)
{
    if (syscall_num < MAX_SYSCALLS) {
        syscall_table[syscall_num] = handler;
    }
}

/**
 * Main system call dispatcher (called from assembly interrupt handler)
 */
int32_t syscall_dispatch_c(struct syscall_context *ctx)
{
    // Validate system call number
    if (ctx->eax >= MAX_SYSCALLS) {
        errno = ENOSYS;
        return -1;
    }
    
    // Get handler for this system call
    syscall_handler_t handler = syscall_table[ctx->eax];
    if (!handler) {
        errno = ENOSYS;
        vga_print("Unimplemented system call: ");
        vga_print_decimal(ctx->eax);
        vga_print("\n");
        return -1;
    }
    
    // Clear errno before system call
    errno = 0;
    
    // Call the system call handler
    int32_t result = handler(ctx);
    
    // Return result (errno is set by individual handlers if needed)
    return result;
}

/**
 * Simple system call dispatcher (called from simplified assembly handler)
 * Takes register values directly as parameters
 */
int32_t syscall_dispatch_simple(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    // Create context structure
    struct syscall_context ctx;
    ctx.eax = syscall_num;
    ctx.ebx = arg1;
    ctx.ecx = arg2;
    ctx.edx = arg3;
    ctx.esi = arg4;
    ctx.edi = arg5;
    ctx.ebp = 0; // Not used
    
    // Call the main dispatcher
    return syscall_dispatch_c(&ctx);
}

/**
 * Legacy system call dispatcher (for backward compatibility)
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

// ========== NEW SYSTEM CALL IMPLEMENTATIONS ==========

/**
 * Read from file descriptor
 */
int32_t sys_read(struct syscall_context *ctx)
{
    int fd = (int)ctx->ebx;
    void *buf = (void *)ctx->ecx;
    size_t count = (size_t)ctx->edx;
    
    // Validate parameters
    if (!syscall_validate_pointer(buf, count)) {
        errno = EFAULT;
        return -1;
    }
    
    // For now, only support reading from stdin (fd 0)
    if (fd == 0) {
        // Read from keyboard
        char *buffer = (char *)buf;
        for (size_t i = 0; i < count; i++) {
            buffer[i] = keyboard_getchar();
            if (buffer[i] == '\n') {
                return i + 1;
            }
        }
        return count;
    }
    
    errno = EBADF;
    return -1;
}

/**
 * Write to file descriptor
 */
int32_t sys_write(struct syscall_context *ctx)
{
    int fd = (int)ctx->ebx;
    const void *buf = (const void *)ctx->ecx;
    size_t count = (size_t)ctx->edx;
    
    // Validate parameters
    if (!syscall_validate_pointer((void *)buf, count)) {
        errno = EFAULT;
        return -1;
    }
    
    // For now, only support writing to stdout (fd 1) and stderr (fd 2)
    if (fd == 1 || fd == 2) {
        const char *buffer = (const char *)buf;
        for (size_t i = 0; i < count; i++) {
            vga_putchar(buffer[i]);
        }
        return count;
    }
    
    errno = EBADF;
    return -1;
}

/**
 * Open file
 */
int32_t sys_open(struct syscall_context *ctx)
{
    const char *pathname = (const char *)ctx->ebx;
    int flags = (int)ctx->ecx;
    int mode = (int)ctx->edx;
    
    // Validate pathname
    if (!syscall_validate_string(pathname, 4096)) {
        errno = EFAULT;
        return -1;
    }
    
    // File system not implemented yet
    errno = ENOSYS;
    return -1;
}

/**
 * Close file descriptor
 */
int32_t sys_close(struct syscall_context *ctx)
{
    int fd = (int)ctx->ebx;
    
    // Standard file descriptors cannot be closed
    if (fd >= 0 && fd <= 2) {
        errno = EINVAL;
        return -1;
    }
    
    // File system not implemented yet
    errno = ENOSYS;
    return -1;
}

/**
 * Yield CPU to other processes
 */
int32_t sys_yield(struct syscall_context *ctx)
{
    // Trigger scheduler
    schedule();
    return 0;
}

/**
 * Sleep for specified number of ticks
 */
int32_t sys_sleep(struct syscall_context *ctx)
{
    uint32_t ticks = ctx->ebx;
    
    if (!current_process) {
        errno = ESRCH;
        return -1;
    }
    
    // For now, just yield - proper sleep would require timer integration
    vga_print("Process sleeping for ");
    vga_print_decimal(ticks);
    vga_print(" ticks\n");
    
    schedule();
    return 0;
}

/**
 * Change data segment size (for malloc implementation)
 */
int32_t sys_brk(struct syscall_context *ctx)
{
    void *addr = (void *)ctx->ebx;
    
    // For now, return current break (not implemented)
    errno = ENOSYS;
    return -1;
}

/**
 * Memory map
 */
int32_t sys_mmap(struct syscall_context *ctx)
{
    void *addr = (void *)ctx->ebx;
    size_t length = (size_t)ctx->ecx;
    int prot = (int)ctx->edx;
    int flags = (int)ctx->esi;
    
    // Memory mapping not implemented yet
    errno = ENOSYS;
    return -1;
}

/**
 * Memory unmap
 */
int32_t sys_munmap(struct syscall_context *ctx)
{
    void *addr = (void *)ctx->ebx;
    size_t length = (size_t)ctx->ecx;
    
    // Memory unmapping not implemented yet
    errno = ENOSYS;
    return -1;
}

/**
 * Allocate memory (kernel malloc wrapper)
 */
int32_t sys_malloc_syscall(struct syscall_context *ctx)
{
    size_t size = (size_t)ctx->ebx;
    
    void *ptr = advanced_kmalloc(size);
    if (!ptr) {
        errno = ENOMEM;
        return 0;
    }
    
    return (int32_t)ptr;
}

// ========== UTILITY FUNCTIONS ==========

/**
 * Validate that a pointer is accessible
 */
bool syscall_validate_pointer(void *ptr, size_t size)
{
    // Basic null check
    if (!ptr) {
        return false;
    }
    
    // Check for reasonable address ranges (basic validation)
    uintptr_t addr = (uintptr_t)ptr;
    
    // Reject obviously bad addresses
    if (addr < 0x1000) {
        return false;  // Null page
    }
    
    if (addr >= 0xC0000000) {
        return false;  // Kernel space
    }
    
    // Check for overflow
    if (addr + size < addr) {
        return false;
    }
    
    return true;
}

/**
 * Validate that a string is accessible and null-terminated
 */
bool syscall_validate_string(const char *str, size_t max_len)
{
    if (!str) {
        return false;
    }
    
    // Basic address validation
    if (!syscall_validate_pointer((void *)str, 1)) {
        return false;
    }
    
    // Check for null termination within max_len
    for (size_t i = 0; i < max_len; i++) {
        if (str[i] == '\0') {
            return true;
        }
    }
    
    return false;  // String too long or not null-terminated
}

/**
 * Set errno value
 */
void syscall_set_errno(int32_t error)
{
    errno = error;
}

/**
 * Get errno value
 */
int32_t syscall_get_errno(void)
{
    return errno;
}
