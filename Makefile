# Compiler and tools
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld
OBJCOPY = i686-elf-objcopy

# Directories
BOOTLOADER_DIR = bootloader
KERNEL_DIR = kernel
BUILD_DIR = build
ISO_DIR = iso

# Flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(KERNEL_DIR)
ASFLAGS = -f elf32
LDFLAGS = -ffreestanding -O2 -nostdlib

# Source files
BOOTLOADER_SRC = $(BOOTLOADER_DIR)/boot.asm
KERNEL_SRC = $(wildcard $(KERNEL_DIR)/*.c) $(wildcard $(KERNEL_DIR)/*/*.c) libc/string.c
KERNEL_SRC := $(filter-out $(KERNEL_DIR)/simple_kernel.c, $(KERNEL_SRC))
KERNEL_ASM = $(wildcard $(KERNEL_DIR)/*.asm) $(wildcard $(KERNEL_DIR)/*/*.asm)

# Object files
KERNEL_OBJ = $(KERNEL_SRC:.c=.o) $(KERNEL_ASM:.asm=.o)

# Output files
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_FILE = $(BUILD_DIR)/simpleos.iso

.PHONY: all clean install-deps run debug print-vars

print-vars:
	@echo "KERNEL_OBJ: $(KERNEL_OBJ)"

all: $(ISO_FILE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(ISO_DIR)/boot/grub

# Build bootloader
$(BUILD_DIR)/boot.bin: $(BOOTLOADER_SRC) | $(BUILD_DIR)
	$(AS) -f bin $(BOOTLOADER_SRC) -o $@

# Build kernel objects
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(KERNEL_OBJ) $(KERNEL_DIR)/linker.ld | $(BUILD_DIR)
	$(CC) -T $(KERNEL_DIR)/linker.ld -o $@ $(LDFLAGS) $(KERNEL_OBJ) -lgcc

# Create ISO image
$(ISO_FILE): $(KERNEL_BIN) | $(BUILD_DIR)
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/
	@if command -v i686-elf-grub-mkrescue >/dev/null 2>&1; then \
		i686-elf-grub-mkrescue -o $@ $(ISO_DIR); \
	elif command -v grub-mkrescue >/dev/null 2>&1; then \
		grub-mkrescue -o $@ $(ISO_DIR); \
	else \
		echo "GRUB not found. Install with: brew install i686-elf-grub"; \
		echo "For now, using raw kernel binary..."; \
		cp $(KERNEL_BIN) $@; \
	fi

# Install dependencies (macOS with Homebrew)
install-deps:
	@echo "Installing cross-compiler and tools..."
	brew install i686-elf-gcc nasm qemu
	@echo "Dependencies installed!"

# Run in QEMU
run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -serial stdio

# Run kernel directly (without GRUB)
run-kernel: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN) -m 128M -serial stdio

# Test with simple kernel
test-simple: 
	i686-elf-gcc -c kernel/simple_kernel.c -o kernel/simple_kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	nasm -f elf32 kernel/boot.asm -o kernel/boot.o
	i686-elf-gcc -T kernel/linker.ld -o build/simple_kernel.bin -ffreestanding -O2 -nostdlib kernel/simple_kernel.o kernel/boot.o -lgcc
	qemu-system-i386 -kernel build/simple_kernel.bin -m 128M

# Debug with GDB
debug: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -m 128M -s -S -serial stdio &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL_BIN)"

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	find . -name "*.o" -type f -delete

# Create bootable USB (be careful with device!)
usb: $(ISO_FILE)
	@echo "Warning: This will overwrite the USB device!"
	@read -p "Enter USB device (e.g., /dev/disk2): " device; \
	sudo dd if=$(ISO_FILE) of=$$device bs=1m

# Show kernel size
size: $(KERNEL_BIN)
	ls -lh $(KERNEL_BIN)
	i686-elf-objdump -h $(KERNEL_BIN)
