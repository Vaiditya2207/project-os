# Getting Started with SimpleOS

Welcome to your operating system kernel project! This is an incredibly ambitious and impressive project that will showcase deep systems programming knowledge.

## What You've Built So Far

Your kernel includes:
- **Bootloader**: Switches from 16-bit to 32-bit mode and loads the kernel
- **VGA Driver**: Text output with colors and scrolling
- **Keyboard Driver**: Interrupt-driven keyboard input
- **Memory Manager**: Basic heap allocation
- **Interrupt System**: IDT setup with PIC remapping

## First Steps

1. **Install Dependencies**:
```bash
make install-deps
```

2. **Build the OS**:
```bash
make all
```

3. **Run in Emulator**:
```bash
make run
```

You should see:
```
Welcome to SimpleOS!
Kernel loaded successfully.

Initializing subsystems...
✓ IDT initialized
✓ Memory management initialized
✓ Keyboard driver initialized

SimpleOS is ready!
Type commands (basic shell coming soon):
> 
```

## Understanding the Code

### Bootloader (`bootloader/boot.asm`)
- Written in assembly language
- Handles the transition from BIOS to your kernel
- Sets up Global Descriptor Table (GDT)
- Switches CPU to protected mode

### Kernel Entry (`kernel/kernel.c`)
- Main kernel function that runs after bootloader
- Initializes all subsystems
- Contains the main kernel loop

### VGA Driver (`kernel/drivers/vga.c`)
- Manages text output to screen
- Handles colors, scrolling, and cursor positioning
- Memory-mapped I/O to VGA buffer

## Next Development Steps

### Immediate (1-2 weeks)
1. **Test the Build**: Make sure everything compiles and runs
2. **Experiment**: Modify the welcome message, change colors
3. **Add Debug Output**: Implement a simple printf function

### Short Term (1-2 months)
1. **Better Memory Management**: Implement proper malloc/free
2. **Timer Driver**: Add system clock for scheduling
3. **Simple Shell**: Basic command processing

### Medium Term (3-6 months)
1. **Process Management**: Task switching and scheduling
2. **File System**: Basic file operations
3. **More Drivers**: Mouse, disk controller

## Debugging Tips

- **QEMU Monitor**: Ctrl+Alt+2 to access QEMU console
- **Serial Debug**: Add serial port output for debugging
- **GDB Integration**: Use `make debug` for step-by-step debugging

## Learning Resources

- [OSDev Wiki](https://wiki.osdev.org/) - The bible of OS development
- [Intel Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html) - x86 reference
- [Linux Kernel](https://github.com/torvalds/linux) - Study real implementations

## Why This Project is Resume Gold

1. **Demonstrates Deep Knowledge**: Shows you understand computers at the lowest level
2. **Rare Skill**: Very few developers have built an OS from scratch
3. **Problem-Solving**: Debugging without standard tools
4. **Architecture Understanding**: Memory management, interrupts, hardware interfaces
5. **Long-term Commitment**: Shows ability to work on complex, multi-year projects

This project can grow into a complete operating system over time, becoming a centerpiece of your portfolio that demonstrates world-class systems programming expertise.

**Good luck on your OS development journey!** 🚀
