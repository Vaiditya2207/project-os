; context_switch.asm - Assembly routines for context switching
; Copyright (c) 2025 SimpleOS

[BITS 32]
section .text

; External functions we might call
extern scheduler_get_next
extern current_process
extern scheduler_tick

; Global symbols exported to C code
global context_switch
global save_context
global restore_context
global switch_to_process

; Structure offsets for process_t (must match process.h)
; cpu_state_t is at offset 20 in process_t structure
CPU_STATE_OFFSET equ 20

; CPU state structure offsets (must match cpu_state_t in process.h)
EAX_OFFSET equ 0
EBX_OFFSET equ 4
ECX_OFFSET equ 8
EDX_OFFSET equ 12
ESI_OFFSET equ 16
EDI_OFFSET equ 20
ESP_OFFSET equ 24
EBP_OFFSET equ 28
EIP_OFFSET equ 32
EFLAGS_OFFSET equ 36
CS_OFFSET equ 40
DS_OFFSET equ 44
ES_OFFSET equ 48
FS_OFFSET equ 52
GS_OFFSET equ 56
SS_OFFSET equ 60

;
; Save current process context
; void save_context(process_t *process)
;
save_context:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    
    ; Get process pointer from parameter
    mov ebx, [ebp + 8]          ; process_t *process
    add ebx, CPU_STATE_OFFSET   ; Point to cpu_state_t
    
    ; Save general purpose registers
    mov [ebx + EAX_OFFSET], eax
    mov eax, [esp + 8]          ; Get original EBX from stack
    mov [ebx + EBX_OFFSET], eax
    mov [ebx + ECX_OFFSET], ecx
    mov [ebx + EDX_OFFSET], edx
    mov [ebx + ESI_OFFSET], esi
    mov [ebx + EDI_OFFSET], edi
    
    ; Save stack pointers (current ESP and EBP)
    mov eax, esp
    add eax, 16                 ; Adjust for pushed values
    mov [ebx + ESP_OFFSET], eax
    mov eax, [ebp]              ; Get original EBP
    mov [ebx + EBP_OFFSET], eax
    
    ; Save instruction pointer (return address)
    mov eax, [ebp + 4]
    mov [ebx + EIP_OFFSET], eax
    
    ; Save flags register
    pushf
    pop eax
    mov [ebx + EFLAGS_OFFSET], eax
    
    ; Save segment registers
    mov ax, cs
    mov [ebx + CS_OFFSET], ax
    mov ax, ds
    mov [ebx + DS_OFFSET], ax
    mov ax, es
    mov [ebx + ES_OFFSET], ax
    mov ax, fs
    mov [ebx + FS_OFFSET], ax
    mov ax, gs
    mov [ebx + GS_OFFSET], ax
    mov ax, ss
    mov [ebx + SS_OFFSET], ax
    
    pop ebx
    pop eax
    pop ebp
    ret

;
; Restore process context
; void restore_context(process_t *process)
;
restore_context:
    ; Get process pointer
    mov ebx, [esp + 4]          ; process_t *process
    add ebx, CPU_STATE_OFFSET   ; Point to cpu_state_t
    
    ; Restore segment registers
    mov ax, [ebx + DS_OFFSET]
    mov ds, ax
    mov ax, [ebx + ES_OFFSET]
    mov es, ax
    mov ax, [ebx + FS_OFFSET]
    mov fs, ax
    mov ax, [ebx + GS_OFFSET]
    mov gs, ax
    mov ax, [ebx + SS_OFFSET]
    mov ss, ax
    
    ; Restore stack pointer
    mov esp, [ebx + ESP_OFFSET]
    
    ; Restore flags
    mov eax, [ebx + EFLAGS_OFFSET]
    push eax
    popf
    
    ; Restore general purpose registers
    mov eax, [ebx + EAX_OFFSET]
    mov ecx, [ebx + ECX_OFFSET]
    mov edx, [ebx + EDX_OFFSET]
    mov esi, [ebx + ESI_OFFSET]
    mov edi, [ebx + EDI_OFFSET]
    mov ebp, [ebx + EBP_OFFSET]
    
    ; Jump to the saved instruction pointer
    jmp [ebx + EIP_OFFSET]

;
; Full context switch between processes
; void context_switch(process_t *old_process, process_t *new_process)
;
context_switch:
    push ebp
    mov ebp, esp
    
    ; Save current process context if old_process is not NULL
    mov eax, [ebp + 8]          ; old_process
    test eax, eax
    jz .no_save
    
    push eax
    call save_context
    add esp, 4
    
.no_save:
    ; Restore new process context
    mov eax, [ebp + 12]         ; new_process
    push eax
    call restore_context
    add esp, 4
    
    pop ebp
    ret

;
; Switch to a specific process (used by scheduler)
; void switch_to_process(process_t *new_process)
;
switch_to_process:
    ; Get the new process
    mov eax, [esp + 4]
    
    ; Update current_process global variable
    mov [current_process], eax
    
    ; Perform the context switch
    push eax                    ; new_process
    push dword [current_process] ; old_process (might be same)
    call context_switch
    add esp, 8
    
    ret

;
; Timer interrupt handler for preemptive scheduling
; Called from IDT when timer fires
;
global timer_interrupt_handler
timer_interrupt_handler:
    ; Save all registers
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Set up kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    
    ; Call C scheduler function
    call scheduler_tick
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    ; Return from interrupt
    iret
