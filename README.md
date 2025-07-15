# SimpleOS - Custom Operating System Kernel

A minimal operating system kernel built from scratch, demonstrating low-level systems programming and computer architecture concepts.

## Features

### Core Components
- [x] Bootloader (16-bit to 32-bit transition)
- [ ] Memory Management (Paging, Heap Allocation)
- [ ] Process Scheduling & Context Switching
- [ ] Basic Filesystem
- [ ] Device Drivers (Keyboard, VGA)
- [ ] System Call Interface

### Future Expansions
- [ ] Shell/CLI Interface
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
├── kernel/            # Core kernel implementation
│   ├── arch/          # Architecture-specific code
│   ├── drivers/       # Device drivers
│   ├── fs/           # Filesystem implementation
│   ├── mem/          # Memory management
│   └── proc/         # Process management
├── libc/             # Standard library implementation
├── tools/            # Build tools and utilities
└── docs/             # Documentation
```

## Learning Resources

- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel Software Developer Manuals](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [AMD64 Architecture Manual](https://www.amd.com/system/files/TechDocs/40332.pdf)

## License

MIT License - Feel free to learn from and build upon this code!
