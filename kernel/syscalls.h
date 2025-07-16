#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel.h"
#include "proc/process.h"

// System call numbers
#define SYS_EXIT 0
#define SYS_FORK 1
#define SYS_EXEC 2
#define SYS_WAIT 3
#define SYS_GETPID 4
#define SYS_KILL 5
#define SYS_READ 6
#define SYS_WRITE 7
#define SYS_OPEN 8
#define SYS_CLOSE 9
#define SYS_YIELD 10
#define SYS_SLEEP 11
#define SYS_BRK 12
#define SYS_MMAP 13
#define SYS_MUNMAP 14
#define SYS_MALLOC 15

#define MAX_SYSCALLS 16

// Error codes (errno values)
#define ESUCCESS        0   // Success
#define EPERM           1   // Operation not permitted  
#define ENOENT          2   // No such file or directory
#define ESRCH           3   // No such process
#define EINTR           4   // Interrupted system call
#define EIO             5   // I/O error
#define ENXIO           6   // No such device or address
#define E2BIG           7   // Argument list too long
#define ENOEXEC         8   // Exec format error
#define EBADF           9   // Bad file number
#define ECHILD          10  // No child processes
#define EAGAIN          11  // Try again
#define ENOMEM          12  // Out of memory
#define EACCES          13  // Permission denied
#define EFAULT          14  // Bad address
#define EINVAL          22  // Invalid argument
#define ENOSYS          38  // Function not implemented

// System call context structure (matches assembly layout)
struct syscall_context {
    uint32_t eax;   // System call number
    uint32_t ebx;   // First argument
    uint32_t ecx;   // Second argument
    uint32_t edx;   // Third argument
    uint32_t esi;   // Fourth argument
    uint32_t edi;   // Fifth argument
    uint32_t ebp;   // Sixth argument
};

// System call function pointer type
typedef int32_t (*syscall_handler_t)(struct syscall_context *ctx);

// Global errno variable
extern int32_t errno;

// Function declarations
void syscall_init(void);
int32_t syscall_dispatch_c(struct syscall_context *ctx);
int32_t syscall_dispatch_simple(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
void syscall_register(uint32_t syscall_num, syscall_handler_t handler);

// Assembly interrupt handlers
extern void syscall_interrupt_handler(void);
extern void syscall_interrupt_handler_simple(void);
extern void syscall_interrupt_handler_debug(void);

// Legacy interface (for compatibility)
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// Individual system calls
void sys_exit(int exit_code);
uint32_t sys_fork(void);
int sys_exec(const char *program, char *const argv[]);
uint32_t sys_wait(uint32_t *status);
uint32_t sys_getpid(void);
int sys_kill(uint32_t pid, int signal);

// New system calls
int32_t sys_read(struct syscall_context *ctx);
int32_t sys_write(struct syscall_context *ctx);
int32_t sys_open(struct syscall_context *ctx);
int32_t sys_close(struct syscall_context *ctx);
int32_t sys_yield(struct syscall_context *ctx);
int32_t sys_sleep(struct syscall_context *ctx);
int32_t sys_brk(struct syscall_context *ctx);
int32_t sys_mmap(struct syscall_context *ctx);
int32_t sys_munmap(struct syscall_context *ctx);
int32_t sys_malloc_syscall(struct syscall_context *ctx);

// Utility functions
bool syscall_validate_pointer(void *ptr, size_t size);
bool syscall_validate_string(const char *str, size_t max_len);
void syscall_set_errno(int32_t error);
int32_t syscall_get_errno(void);

// User-space system call interface (inline functions for user programs)
static inline int32_t syscall0(uint32_t syscall_num) {
    int32_t result;
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num)
        : "memory"
    );
    return result;
}

static inline int32_t syscall1(uint32_t syscall_num, uint32_t arg1) {
    int32_t result;
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num), "b" (arg1)
        : "memory"
    );
    return result;
}

static inline int32_t syscall2(uint32_t syscall_num, uint32_t arg1, uint32_t arg2) {
    int32_t result;
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num), "b" (arg1), "c" (arg2)
        : "memory"
    );
    return result;
}

static inline int32_t syscall3(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int32_t result;
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    return result;
}

// Convenience macros for common system calls
#define exit(code)          syscall1(SYS_EXIT, code)
#define fork()              syscall0(SYS_FORK)
#define getpid()            syscall0(SYS_GETPID)
#define yield()             syscall0(SYS_YIELD)
#define sleep(seconds)      syscall1(SYS_SLEEP, seconds)

#endif
