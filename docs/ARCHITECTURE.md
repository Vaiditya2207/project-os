# SimpleOS Architecture Overview

## System Architecture

SimpleOS follows a monolithic kernel architecture where all kernel services run in kernel space with full hardware access. The current implementation includes a complete process management system with preemptive multitasking.

```
┌─────────────────────────────────────┐
│        User Applications            │
│     (Demo Processes, Shell)         │
├─────────────────────────────────────┤
│         System Call Interface      │
│           (Future Feature)          │
├─────────────────────────────────────┤
│                                     │
│           Kernel Space              │
│  ┌─────────────┬─────────────────┐   │
│  │   Process   │     Memory      │   │
│  │ Management  │   Management    │   │
│  │             │   (kmalloc)     │   │
│  ├─────────────┼─────────────────┤   │
│  │   Device    │   Interactive   │   │
│  │   Drivers   │     Shell       │   │
│  │(VGA,KB,TMR) │   (Commands)    │   │
│  └─────────────┴─────────────────┘   │
├─────────────────────────────────────┤
│          Hardware Layer             │
│    (Timer, Keyboard, VGA, CPU)      │
└─────────────────────────────────────┘
```

## Process Management Architecture

### Process Control Block (PCB)
Each process is represented by a `process_t` structure containing:
- **Process State**: Ready, Running, Blocked, Terminated
- **CPU Context**: Complete register state (EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP, EFLAGS)
- **Memory Layout**: Stack base, heap base, memory usage tracking
- **Scheduling Info**: Priority, time slice, runtime statistics
- **Process Relationships**: Parent/child relationships, process tree

### Context Switching
Implemented in assembly (`kernel/arch/context_switch.asm`):
1. **Save Context**: Store all CPU registers and flags to PCB
2. **Switch Stack**: Update ESP to new process stack
3. **Restore Context**: Load new process state from PCB
4. **Resume Execution**: Jump to new process instruction pointer

### Scheduler
Round-robin preemptive scheduler with timer-based task switching:
- **Ready Queue**: Doubly-linked list of ready processes
- **Time Slicing**: Each process gets equal CPU time
- **Preemption**: Timer interrupts trigger context switches
- **Priority Support**: High, Normal, Low priority levels

### Process Creation
Dynamic process creation with memory allocation:
1. **Allocate PCB**: kmalloc() for process control block
2. **Setup Stack**: 4KB stack per process with guard pages
3. **Initialize Context**: Set entry point, stack pointer, and flags
4. **Add to Scheduler**: Insert into ready queue for execution

## Memory Layout

```
0xFFFFFFFF ┌─────────────────┐
           │   Kernel Space  │ (3GB-4GB)
0xC0000000 ├─────────────────┤
           │   User Space    │ (0-3GB)
           │                 │
           │                 │
0x00100000 ├─────────────────┤ <- Kernel loaded here
           │   BIOS/VGA      │
0x000A0000 ├─────────────────┤
           │   Free Memory   │
0x00001000 ├─────────────────┤
           │   Bootloader    │
0x00007C00 ├─────────────────┤
           │   BIOS Data     │
0x00000000 └─────────────────┘
```

## Interrupt Handling

The kernel uses the Interrupt Descriptor Table (IDT) to handle hardware and software interrupts.

### Interrupt Types
- **Hardware Interrupts (IRQ)**: Keyboard, timer, disk
- **Software Interrupts**: System calls, exceptions
- **CPU Exceptions**: Page faults, division by zero

### PIC Remapping
- Master PIC: IRQ 0-7 → Interrupts 32-39
- Slave PIC: IRQ 8-15 → Interrupts 40-47

## Device Drivers

### Enhanced VGA Driver (`kernel/drivers/vga.c`)
- **Text Mode**: 80x25 character display with 16 colors
- **Memory-Mapped I/O**: Direct access to VGA buffer at 0xB8000
- **Scrolling Support**: Automatic screen scrolling when buffer is full
- **Cursor Management**: Visual cursor positioning and movement
- **Color Support**: Foreground/background color combinations
- **Special Characters**: Newline, carriage return, tab, backspace handling

### Advanced Keyboard Driver (`kernel/drivers/keyboard.c`)
- **Full ASCII Support**: Complete character set including special symbols
- **Modifier Keys**: Shift, Caps Lock, Ctrl, Alt key handling
- **Key Repeat**: Progressive key repeat with acceleration
- **Scancode Translation**: Hardware scancode to ASCII conversion
- **Input Buffer**: 256-character input buffer with overflow protection
- **State Management**: Persistent modifier key states

### System Timer Driver (`kernel/drivers/timer.c`)
- **PIT Configuration**: Programmable Interval Timer setup
- **Interrupt Generation**: Regular timer interrupts for scheduler
- **Frequency Control**: Configurable timer frequency (default 100Hz)
- **Scheduler Integration**: Automatic preemptive task switching

## Memory Management

### Current Implementation
- **Heap Allocator**: kmalloc/kfree with basic free list management
- **Process Stacks**: 4KB per-process stacks with stack overflow protection
- **Memory Tracking**: Usage statistics and debugging information
- **Static Memory**: Kernel data structures in static memory areas

### Memory Layout
```
0xFFFFFFFF ┌─────────────────┐
           │   Kernel Space  │ (3GB-4GB)
           │   (Future)      │
0xC0000000 ├─────────────────┤
           │   Process Heap  │
           │   & Stacks      │
0x00200000 ├─────────────────┤
           │   Kernel Heap   │
0x00100000 ├─────────────────┤ <- Kernel loaded here
           │   BIOS/VGA      │
0x000A0000 ├─────────────────┤
           │   Free Memory   │
0x00001000 ├─────────────────┤
           │   Bootloader    │
0x00007C00 ├─────────────────┤
           │   BIOS Data     │
0x00000000 └─────────────────┘
```

### Future Improvements
- **Paging**: Virtual memory management with page tables
- **User/Kernel Separation**: Memory protection between privilege levels
- **Advanced Allocators**: Slab allocator, buddy system

## Process Management

### Future Implementation
- **Process Control Blocks (PCB)**: Store process state
- **Context Switching**: Save/restore CPU registers
- **Scheduling**: Round-robin or priority-based
- **Address Spaces**: Virtual memory per process

## File System

### Planned Features
- **Simple File Allocation Table**: Track file locations
- **Directory Structure**: Hierarchical organization
- **Basic Operations**: Create, read, write, delete
- **Disk Interface**: IDE/SATA controller driver

## System Calls

### Interface Design
```c
// Example system call interface
int sys_read(int fd, void* buf, size_t count);
int sys_write(int fd, const void* buf, size_t count);
int sys_open(const char* pathname, int flags);
int sys_close(int fd);
```

### Implementation
- **INT 0x80**: Software interrupt for system calls
- **Parameter Passing**: Registers (EAX=syscall number, EBX/ECX/EDX=args)
- **Return Values**: EAX register

## Performance Considerations

### Optimization Strategies
- **Minimal Context Switches**: Reduce kernel/user transitions
- **Efficient Memory Access**: Cache-friendly data structures
- **Interrupt Latency**: Fast interrupt handlers
- **Assembly Optimization**: Critical path optimization

## Security Model

### Current Security
- **Ring 0**: All code runs in kernel mode (no protection)

### Future Security Features
- **Ring 3**: User mode with restricted access
- **Memory Protection**: Page-level permissions
- **System Call Validation**: Parameter checking
- **Process Isolation**: Separate address spaces
