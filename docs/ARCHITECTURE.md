# Architecture Overview

## System Architecture

SimpleOS follows a monolithic kernel architecture where all kernel services run in kernel space with full hardware access.

```
┌─────────────────────────────────────┐
│           User Applications         │
├─────────────────────────────────────┤
│         System Call Interface      │
├─────────────────────────────────────┤
│                                     │
│           Kernel Space              │
│  ┌─────────────┬─────────────────┐   │
│  │   Process   │     Memory      │   │
│  │ Management  │   Management    │   │
│  ├─────────────┼─────────────────┤   │
│  │   Device    │   File System   │   │
│  │   Drivers   │                 │   │
│  └─────────────┴─────────────────┘   │
├─────────────────────────────────────┤
│          Hardware Layer             │
└─────────────────────────────────────┘
```

## Boot Process

1. **BIOS/UEFI**: Power-on self-test, hardware initialization
2. **Bootloader**: Load kernel from disk, switch to protected mode
3. **Kernel**: Initialize subsystems, start user processes

### Detailed Boot Sequence

```
Power On → BIOS → MBR (Bootloader) → Kernel → Init → Shell
```

1. **BIOS** loads first 512 bytes (bootloader) from disk to 0x7C00
2. **Bootloader** switches from 16-bit real mode to 32-bit protected mode
3. **Kernel** takes control, initializes hardware and subsystems
4. **Init process** starts user-space environment
5. **Shell** provides user interface

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

### VGA Driver
- Text mode 80x25 characters
- 16 colors (4-bit color depth)
- Memory-mapped I/O at 0xB8000

### Keyboard Driver
- PS/2 keyboard controller
- Scancode to ASCII translation
- Interrupt-driven input (IRQ1)

## Memory Management

### Current Implementation
- **Bump Allocator**: Simple linear allocation
- **Fixed Heap**: 1MB heap starting at 0x100000

### Future Improvements
- **Paging**: Virtual memory management
- **Free Lists**: Proper malloc/free implementation
- **Memory Protection**: User/kernel space separation

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
