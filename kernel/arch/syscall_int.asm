; System Call Interrupt Handler (INT 0x80) - Working Version
; This file implements the low-level system call interface

[BITS 32]

global syscall_interrupt_handler_simple
global syscall_interrupt_handler_debug
extern syscall_dispatch_simple

section .text

; Proper system call interrupt handler (based on working timer interrupt)
syscall_interrupt_handler_simple:
    ; Save all registers (like timer interrupt)
    pusha               ; Save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs
    
    ; Set up kernel data segment (like timer interrupt)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    
    ; For now, just test if we can get here and return
    ; TODO: Call C dispatcher with register values
    
    ; No EOI needed for software interrupt (INT 0x80)
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    iret                ; Return from interrupt

; Debug version that's slightly more complex but still minimal
global syscall_interrupt_handler_debug
syscall_interrupt_handler_debug:
    ; Save only EAX and test basic functionality
    push eax
    
    ; Set a test return value based on the syscall number
    cmp eax, 4             ; SYS_GETPID
    je .getpid
    
    ; Default case - return error
    mov eax, 0xFFFFFFFF
    jmp .done
    
.getpid:
    mov eax, 1             ; Return PID 1
    
.done:
    ; Don't restore original EAX - we want our return value
    add esp, 4             ; Remove the pushed EAX
    iret
