# SimpleOS - Custom Operating System Kernel

A minimal operating system kernel built from scratch, demonstrating low-level systems programming and computer architecture concepts.

## Features

### Core Components
- [x] Bootloader (16-bit to 32-bit transition)
- [x] Memory Management (Basic heap allocation with kmalloc/kfree)
- [x] Process Management (PCB, Process creation, Context switching)
- [x] Basic Scheduler (Round-robin with preemptive multitasking)
- [x] Device Drivers (Enhanced keyboard with repeat, VGA with scrolling)
- [x] Timer Driver (PIT-based system timer)
- [ ] Basic Filesystem
- [ ] System Call Interface

### Implemented Features
- [x] **Interactive Shell** with command processing
- [x] **Enhanced Keyboard Driver** (Full ASCII, special chars, key repeat, modifiers)
- [x] **VGA Display Driver** (Text mode, colors, scrolling, cursor)
- [x] **Process Management** (Process creation, PCB, context switching)
- [x] **Preemptive Scheduler** (Round-robin, timer-based task switching)
- [x] **Memory Management** (Basic heap with kmalloc/kfree)
- [x] **Timer Driver** (PIT-based system timer for scheduling)

### Commands Available
- `help` - Show available commands
- `about` - System information and credits
- `clear` - Clear screen
- `version` - Detailed version information
- `keytest` - Test keyboard functionality
- `meminfo` - Display memory usage
- `ps` - List processes
- `create <name>` - Create a test process
- `kill <pid>` - Terminate a process

### Future Expansions
- [ ] System Call Interface
- [ ] Virtual File System
- [ ] Network Stack
- [ ] GUI System
- [ ] More Device Drivers
- [ ] POSIX Compliance

## Build Requirements

- GCC Cross-Compiler (i686-elf)
- NASM Assembler
- QEMU Emulator
- Make

## Quick Start

```bash
# Install dependencies (macOS)
make install-deps

# Build the OS
make all

# Run in QEMU
make run

# Debug with GDB
make debug
```

## Project Structure

```
├── bootloader/         # Boot sequence and kernel loading
│   └── boot.asm       # Master Boot Record and kernel loader
├── kernel/            # Core kernel implementation
│   ├── arch/          # Architecture-specific code (context switching)
│   ├── drivers/       # Device drivers (VGA, keyboard, timer)
│   ├── mem/           # Memory management (heap allocation)
│   ├── proc/          # Process management (scheduler, PCB)
│   ├── kernel.c       # Main kernel and interactive shell
│   └── kernel.h       # Core kernel headers and definitions
├── libc/             # Standard library implementation (string functions)
├── build/            # Compiled binaries and ISO images
└── docs/             # Documentation and architecture guides
```

## Key Components

### Bootloader (`bootloader/boot.asm`)
- **Real Mode to Protected Mode**: 16-bit to 32-bit transition
- **Kernel Loading**: Loads kernel from disk sectors
- **GDT Setup**: Global Descriptor Table for memory segmentation
- **A20 Line**: Enables access to extended memory

### Kernel Core (`kernel/kernel.c`)
- **Interactive Shell**: Command processing and user interaction
- **System Initialization**: Hardware setup and driver initialization
- **Command Handling**: Built-in commands (help, ps, meminfo, etc.)

### Process Management (`kernel/proc/`)
- **Process Control Blocks**: Complete process state management
- **Context Switching**: Assembly-optimized CPU state saving/restoring
- **Round-Robin Scheduler**: Preemptive multitasking with timer interrupts
- **Process Creation**: Dynamic process spawning with memory allocation

### Device Drivers (`kernel/drivers/`)
- **Enhanced Keyboard**: Full ASCII support, key repeat, modifier keys
- **VGA Display**: Text mode, colors, scrolling, cursor management
- **System Timer**: PIT-based timing for scheduler preemption

### Memory Management (`kernel/mem/`)
- **Heap Allocator**: kmalloc/kfree implementation
- **Stack Management**: Per-process stack allocation
- **Memory Tracking**: Usage monitoring and debugging

## Learning Resources

- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel Software Developer Manuals](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [AMD64 Architecture Manual](https://www.amd.com/system/files/TechDocs/40332.pdf)

## License

MIT License - Feel free to learn from and build upon this code!
