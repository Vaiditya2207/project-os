# SimpleOS Development Guide

## Development Environment Setup

### macOS Setup (Recommended)

1. **Install Homebrew** (if not already installed):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. **Install Cross-Compiler Toolchain**:
```bash
brew install i686-elf-gcc
brew install nasm
brew install qemu
```

3. **Install Additional Tools**:
```bash
brew install grub         # For creating bootable ISOs
brew install hexdump      # For debugging binary files
```

## Building and Testing

### Basic Build Commands
```bash
# Clean previous builds
make clean

# Build everything (kernel + bootloader + ISO)
make all

# Run in QEMU emulator (recommended)
make run

# Debug with GDB (advanced)
make debug
```

### Development Workflow
1. **Edit source code** in kernel/, bootloader/, or libc/
2. **Build** with `make all`
3. **Test** with `make run` - boots into SimpleOS shell
4. **Test features** using built-in commands:
   - Try `help` for available commands
   - Test process management with `ps`, `create`, `kill`
   - Test keyboard with `keytest`
   - Check memory usage with `meminfo`
5. **Debug** issues with `make debug` or printf-style debugging

## Current Project Structure

```
├── bootloader/         # Bootstrap code
│   └── boot.asm       # Master Boot Record and kernel loader
├── kernel/            # Kernel source code
│   ├── arch/          # Architecture-specific code
│   │   ├── context_switch.asm  # Process context switching
│   │   ├── idt.c      # Interrupt Descriptor Table
│   │   └── interrupts.asm     # Interrupt handlers
│   ├── drivers/       # Hardware drivers
│   │   ├── vga.c      # Enhanced VGA driver (colors, scrolling)
│   │   ├── keyboard.c # Advanced keyboard driver (full ASCII)
│   │   ├── keyboard.h # Keyboard constants and structures
│   │   ├── timer.c    # System timer driver (PIT)
│   │   └── timer.h    # Timer constants and functions
│   ├── proc/          # Process management
│   │   ├── process.c  # Process control, scheduling, context switching
│   │   ├── process.h  # Process structures and definitions
│   │   └── demo_processes.c  # Test processes for scheduler
│   ├── mem/           # Memory management
│   │   └── memory.c   # Heap allocator (kmalloc/kfree)
│   ├── syscalls.c     # System call framework (foundation)
│   ├── syscalls.h     # System call definitions
│   ├── kernel.c       # Main kernel entry and interactive shell
│   ├── kernel.h       # Core kernel headers and definitions
│   └── linker.ld      # Linker script for kernel layout
├── libc/              # Standard library
│   └── string.c       # String manipulation functions
├── build/             # Build artifacts (kernel.bin, simpleos.iso)
├── iso/               # ISO creation directory
├── docs/              # Documentation
└── Makefile           # Build system with multiple targets
```

## Development Areas

### Completed Features (v1.3)
✅ **Process Management**: Full PCB, context switching, round-robin scheduler  
✅ **Memory Management**: Heap allocator with kmalloc/kfree  
✅ **Device Drivers**: Enhanced VGA, advanced keyboard, system timer  
✅ **Interactive Shell**: Command processing with multiple built-in commands  
✅ **Boot System**: Complete bootloader with protected mode transition  

### Next Development Priorities

#### Phase 1: System Calls & User Mode (1-2 months)
1. **System Call Interface**: Implement INT 0x80 handler and syscall dispatcher
2. **User/Kernel Separation**: Add privilege level switching and memory protection
3. **Standard Syscalls**: fork(), exec(), wait(), exit() implementation

#### Phase 2: File System (2-3 months)  
1. **Virtual File System**: Abstract file system interface
2. **Simple FS Implementation**: FAT12 or custom file system
3. **File Operations**: Basic file I/O and directory management

#### Phase 3: Advanced Drivers (1-2 months)
1. **Storage Driver**: IDE/SATA hard disk support
2. **Network Interface**: Basic Ethernet driver
3. **Additional I/O**: Serial port, mouse support

## Debugging Tips

1. **Use QEMU Monitor**: Press `Ctrl+Alt+2` to access QEMU monitor
2. **GDB Integration**: Use `make debug` to attach GDB debugger
3. **Serial Output**: Add serial port debugging for printf-style debugging
4. **Memory Dumps**: Use QEMU's `info registers` and `x` commands

## Common Issues

1. **Boot Fails**: Check bootloader assembly syntax and boot signature
2. **Kernel Panic**: Verify multiboot header and kernel entry point
3. **No Keyboard Input**: Ensure IDT and PIC are properly initialized
4. **Display Issues**: Check VGA memory addressing and color codes

## Resources

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development guide
- [Intel Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html) - x86 architecture reference
- [QEMU Documentation](https://qemu.readthedocs.io/) - Emulator usage guide
