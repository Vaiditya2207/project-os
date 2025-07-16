; Safe Interrupt handlers - v1.2.1

[GLOBAL idt_flush]
[GLOBAL timer_interrupt_wrapper]
[GLOBAL int3_handler_asm]
[EXTERN exception_handler]
[EXTERN timer_handler]
[EXTERN int3_handler]

; Load IDT - Simple and safe
idt_flush:
    mov eax, [esp+4]  ; Get the pointer to the IDT
    lidt [eax]        ; Load the IDT
    ret

; Safe INT 3 handler wrapper
[GLOBAL int3_handler_asm]
int3_handler_asm:
    pusha               ; Save all registers
    push ds
    push es
    push fs
    push gs
    
    ; Set up kernel data segment (using actual segment value)
    mov ax, 0x18
    mov ds, ax
    mov es, ax
    
    call int3_handler   ; Call C handler
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa                
    iret                ; Return from interrupt

; Simple exception handler stub
[GLOBAL exception_handler_asm]
exception_handler_asm:
    pusha               ; Save all registers
    call exception_handler  ; Call C handler
    popa                ; Restore all registers
    iret                ; Return from interrupt

; Timer interrupt wrapper (IRQ 0)
timer_interrupt_wrapper:
    pusha               ; Save all registers
    push ds
    push es
    push fs
    push gs
    
    ; Set up kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    
    ; Call C timer handler
    call timer_handler
    
    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    iret                ; Return from interrupt
