#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel.h"
#include "proc/process.h"

// System call numbers
#define SYS_EXIT    1
#define SYS_FORK    2
#define SYS_EXEC    3
#define SYS_WAIT    4
#define SYS_GETPID  5
#define SYS_KILL    6

// System call interface
void syscall_init(void);
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// Individual system calls
void sys_exit(int exit_code);
uint32_t sys_fork(void);
int sys_exec(const char *program, char *const argv[]);
uint32_t sys_wait(uint32_t *status);
uint32_t sys_getpid(void);
int sys_kill(uint32_t pid, int signal);

#endif
