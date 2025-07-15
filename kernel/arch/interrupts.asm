; Interrupt handlers

[GLOBAL idt_flush]
[GLOBAL keyboard_handler_wrapper]
[EXTERN keyboard_handler_internal]

; Load IDT
idt_flush:
    mov eax, [esp+4]  ; Get the pointer to the IDT
    lidt [eax]        ; Load the IDT
    ret

; Keyboard interrupt handler wrapper
keyboard_handler_wrapper:
    pusha               ; Save all registers
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call keyboard_handler_internal  ; Call C handler
    
    pop gs
    pop fs
    pop es
    pop ds
    popa                ; Restore all registers
    iret                ; Return from interrupt
