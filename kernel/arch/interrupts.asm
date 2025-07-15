; Safe Interrupt handlers - v1.1

[GLOBAL idt_flush]
[EXTERN exception_handler]

; Load IDT - Simple and safe
idt_flush:
    mov eax, [esp+4]  ; Get the pointer to the IDT
    lidt [eax]        ; Load the IDT
    ret

; Simple exception handler stub
[GLOBAL exception_handler_asm]
exception_handler_asm:
    pusha               ; Save all registers
    call exception_handler  ; Call C handler
    popa                ; Restore all registers
    iret                ; Return from interrupt
