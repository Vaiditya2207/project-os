# Getting Started with SimpleOS

Welcome to SimpleOS - a fully functional operating system kernel with process management, interactive shell, and advanced device drivers!

## What You've Built (v1.3)

SimpleOS is now a sophisticated kernel featuring:
- **Complete Bootloader**: 16-bit to 32-bit transition with GDT setup
- **Process Management**: Full PCB implementation with context switching
- **Preemptive Scheduler**: Round-robin scheduling with timer interrupts
- **Advanced Device Drivers**: Enhanced VGA, keyboard with modifiers, system timer
- **Memory Management**: Heap allocator with kmalloc/kfree
- **Interactive Shell**: Command-line interface with multiple built-in commands
- **Context Switching**: Assembly-optimized process state management

## Quick Start

1. **Install Dependencies** (macOS):
```bash
brew install i686-elf-gcc nasm qemu
```

2. **Build SimpleOS**:
```bash
make all
```

3. **Run the Operating System**:
```bash
make run
```

You should see the SimpleOS boot sequence followed by an interactive shell:
```
SimpleOS Bootloader v1.0
Loading kernel...

Welcome to SimpleOS!
Kernel loaded successfully.

Initializing subsystems...
  Memory management...
  Interrupt system (IDT)...
  Keyboard driver...
  Timer driver...
  Process management...
  Scheduler...

SimpleOS is ready!

SimpleOS> _
```

## Exploring SimpleOS Features

### Interactive Shell Commands
Once SimpleOS boots, try these commands:

- **`help`** - Display all available commands
- **`about`** - System information and credits  
- **`version`** - Detailed version and feature information
- **`clear`** - Clear the screen
- **`meminfo`** - Display memory usage statistics
- **`ps`** - List all running processes
- **`create <name>`** - Create a new test process
- **`kill <pid>`** - Terminate a process by ID
- **`keytest`** - Interactive keyboard testing mode

### Testing Process Management
```bash
SimpleOS> ps                    # List current processes
SimpleOS> create myprocess      # Create a new process
SimpleOS> ps                    # See the new process
SimpleOS> kill 2                # Terminate process ID 2
```

### Testing Advanced Keyboard Features
```bash
SimpleOS> keytest               # Enter keyboard test mode
# Try typing with:
# - Shift + letters (uppercase)
# - Shift + numbers (special symbols !@#$%^&*())
# - Caps Lock toggle
# - Special characters []{}|;':"<>,./?`~-=_+
# - Key repeat (hold any key)
# - Tab (4 spaces), Backspace, Enter
```

## Understanding the Architecture

### Process Management (`kernel/proc/`)
SimpleOS implements full process management with:
- **Process Control Blocks (PCB)**: Complete process state storage
- **Context Switching**: Assembly-optimized CPU state saving/restoring
- **Round-Robin Scheduler**: Preemptive multitasking with equal time slices
- **Process Creation**: Dynamic process spawning with memory allocation
- **Process States**: Ready, Running, Blocked, Terminated

### Memory Management (`kernel/mem/`)
- **Heap Allocator**: kmalloc/kfree with free list management
- **Per-Process Stacks**: 4KB stacks with overflow protection
- **Memory Tracking**: Usage statistics and leak detection

### Device Drivers (`kernel/drivers/`)
- **Enhanced VGA**: Colors, scrolling, cursor, special character handling
- **Advanced Keyboard**: Full ASCII, modifiers, key repeat, state management
- **System Timer**: PIT-based interrupts for scheduler preemption

## Next Development Steps

### Immediate Exploration (1-2 days)
1. **Test All Features**: Try every shell command and explore process management
2. **Code Reading**: Study the process management and context switching code
3. **Experimentation**: Modify shell messages, add new commands

### Short Term Enhancements (1-2 months)
1. **System Calls**: Implement INT 0x80 syscall interface
2. **User Mode**: Add privilege level separation (Ring 0/Ring 3)
3. **File System Foundation**: VFS layer and basic file operations
4. **More Process Features**: Process priorities, sleeping, inter-process communication

### Medium Term Goals (3-6 months)
1. **Complete File System**: FAT12 or custom filesystem implementation
2. **Advanced Memory**: Virtual memory with paging, memory protection
3. **Network Driver**: Basic Ethernet support for future networking
4. **Storage Driver**: IDE/SATA hard disk controller

## Why This Project Demonstrates Exceptional Skill

### Technical Achievements
1. **Complete Process Management**: Full PCB, context switching, and preemptive scheduling
2. **Assembly Integration**: Hand-optimized context switching in assembly language
3. **Interrupt Handling**: Timer-based preemption and keyboard interrupt processing
4. **Memory Management**: Custom heap allocator with proper allocation/deallocation
5. **Device Driver Development**: Advanced keyboard and VGA drivers with full feature sets

### Professional Value
1. **Rare Expertise**: Very few developers have built a complete OS with process management
2. **Deep System Knowledge**: Demonstrates understanding of computer architecture at the lowest level
3. **Problem-Solving Skills**: Shows ability to solve complex, low-level programming challenges
4. **Code Quality**: Well-structured, documented, and modular codebase
3. **Problem-Solving**: Debugging without standard tools
4. **Architecture Understanding**: Memory management, interrupts, hardware interfaces
5. **Long-term Commitment**: Shows ability to work on complex, multi-year projects

This project can grow into a complete operating system over time, becoming a centerpiece of your portfolio that demonstrates world-class systems programming expertise.

**Good luck on your OS development journey!** 🚀
