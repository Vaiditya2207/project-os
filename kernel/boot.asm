; Kernel bootstrap assembly
; This file handles the transition from GRUB to our C kernel

[BITS 32]

; Multiboot header constants
MBALIGN     equ  1 << 0              ; align loaded modules on page boundaries
MEMINFO     equ  1 << 1              ; provide memory map
FLAGS       equ  MBALIGN | MEMINFO   ; this is the Multiboot 'flag' field
MAGIC       equ  0x1BADB002          ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)     ; checksum of above, to prove we are multiboot

; Declare a multiboot header that marks the program as a kernel.
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Reserve a stack for the initial thread.
section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

; The kernel entry point.
section .text
global _start:function (_start.end - _start)
_start:
    ; Initialize the stack pointer.
    mov esp, stack_top

    ; Call the global constructors.
    ; call _init

    ; Transfer control to the main kernel.
    extern kernel_main
    call kernel_main

    ; If the kernel returns, hang in an infinite loop.
    cli
.hang:  hlt
    jmp .hang
.end:
