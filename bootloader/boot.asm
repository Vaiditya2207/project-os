; SimpleOS Bootloader
; This bootloader will load our kernel and switch to 32-bit protected mode

[BITS 16]
[ORG 0x7C00]

start:
    ; Clear interrupts and set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Print boot message
    mov si, boot_msg
    call print_string

    ; Load kernel from disk
    call load_kernel

    ; Enable A20 line
    call enable_a20

    ; Set up GDT
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Jump to 32-bit code
    jmp CODE_SEG:protected_mode

; Print string function (16-bit)
print_string:
    mov ah, 0x0E
.next_char:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .next_char
.done:
    ret

; Load kernel from disk
load_kernel:
    mov si, loading_msg
    call print_string

    ; Reset disk
    mov ah, 0x00
    mov dl, 0x80
    int 0x13

    ; Load kernel sectors
    mov ah, 0x02        ; Read sectors
    mov al, 15          ; Number of sectors to read
    mov ch, 0           ; Cylinder
    mov cl, 2           ; Sector (starts from 2, after bootloader)
    mov dh, 0           ; Head
    mov dl, 0x80        ; Drive
    mov bx, 0x1000      ; Load to 0x1000
    int 0x13

    jc disk_error
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    hlt

; Enable A20 line
enable_a20:
    ; Fast A20 method
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; 32-bit protected mode code
[BITS 32]
protected_mode:
    ; Set up data segments
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Jump to kernel
    jmp 0x1000

; GDT (Global Descriptor Table)
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0

    ; Code segment descriptor
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access byte
    db 11001111b    ; Granularity byte
    db 0x00         ; Base (bits 24-31)

    ; Data segment descriptor
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Access byte
    db 11001111b    ; Granularity byte
    db 0x00         ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Offset

; Constants
CODE_SEG equ gdt_start + 0x08
DATA_SEG equ gdt_start + 0x10

; Messages
boot_msg db 'SimpleOS Bootloader v1.0', 0x0D, 0x0A, 0
loading_msg db 'Loading kernel...', 0x0D, 0x0A, 0
disk_error_msg db 'Disk read error!', 0x0D, 0x0A, 0

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
