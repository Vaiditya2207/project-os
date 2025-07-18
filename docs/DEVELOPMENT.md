# Development Guide

## Setting Up the Development Environment

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

3. **Install GRUB** (for creating bootable ISOs):
```bash
brew install grub
```

## Building the OS

```bash
# Clean previous builds
make clean

# Build everything
make all

# Run in QEMU emulator
make run

# Debug with GDB
make debug
```

## Development Workflow

1. **Edit source code** in kernel/, bootloader/, or libc/
2. **Build** with `make all`
3. **Test** with `make run`
4. **Debug** issues with `make debug`

## Project Structure Explained

```
├── bootloader/         # Bootstrap code
│   └── boot.asm       # Assembly bootloader
├── kernel/            # Kernel source code
│   ├── arch/          # Architecture-specific code
│   │   ├── idt.c      # Interrupt handling
│   │   └── idt.asm    # IDT assembly functions
│   ├── drivers/       # Hardware drivers
│   │   ├── vga.c      # VGA text mode driver
│   │   └── keyboard.c # Keyboard input driver
│   ├── mem/           # Memory management
│   │   └── memory.c   # Heap allocation
│   ├── kernel.c       # Main kernel entry point
│   ├── kernel.h       # Kernel headers
│   └── linker.ld      # Linker script
├── libc/              # Standard library
│   └── string.c       # String functions
├── build/             # Build output (generated)
├── iso/               # ISO creation directory
├── Makefile           # Build system
└── grub.cfg           # GRUB configuration
```

## Next Steps for Development

### Phase 1: Core Improvements
1. **Better Memory Management**: Implement proper malloc/free with free lists
2. **Process Management**: Add task switching and basic scheduler
3. **File System**: Implement a simple file system (FAT12 or custom)
4. **More Drivers**: Add timer, mouse, and disk drivers

### Phase 2: Advanced Features
1. **Shell**: Command-line interface with basic commands
2. **User Mode**: Switch between kernel and user space
3. **System Calls**: Implement proper syscall interface
4. **Networking**: Basic TCP/IP stack

### Phase 3: Full OS Features
1. **GUI**: Window manager and graphical interface
2. **Applications**: Text editor, calculator, games
3. **Package Manager**: Software installation system
4. **POSIX Compliance**: Unix-like API compatibility

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
