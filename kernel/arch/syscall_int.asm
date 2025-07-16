; System Call Interrupt Handler (INT 0x80) - Debug Version
; Systematic debugging approach to fix the crash

[BITS 32]

global syscall_interrupt_handler_simple
global syscall_interrupt_handler_debug
global syscall_interrupt_handler_minimal
extern syscall_dispatch_simple

section .text

; Level 1: Absolute minimal handler - just return safely
syscall_interrupt_handler_minimal:
    ; Don't touch ANYTHING - just return with original EAX
    iret

; Level 2: Test basic register modification
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

; Level 3: Full register save/restore (like timer interrupt)
syscall_interrupt_handler_simple:
    ; Save all registers (like timer interrupt)
    pushad              ; Save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs
    
    ; Set up kernel data segment (using actual segment value 0x18)
    mov ax, 0x18
    mov ds, ax
    mov es, ax
    
    ; For now, just test if we can get here and return
    ; TODO: Call C dispatcher with register values
    
    ; Restore registers in reverse order
    pop gs
    pop fs
    pop es
    pop ds
    popad
    
    iret                ; Return from interrupt
