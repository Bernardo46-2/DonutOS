[org 0x7c00]
KERNEL_LOCATION equ 0x8000
mov [BOOT_DISK], dl
jmp _start

print_str:                   ; bx = str
    mov ah, 0x0e             ; code for printing
print_loop:
    mov al, [bx]             ; dereference str
    cmp al, 0                ; compare with null
    je print_end             ; if so, return
    int 0x10                 ; print interrupt
    inc bx                   ; inc str ptr
    jmp print_loop           ; next print iteration
print_end:
    ret

_start:
    xor ax, ax
    mov es, ax               ; es not needed
    mov ds, ax
    mov bp, 0x8000
    mov sp, bp

    mov ah, 2                ; code for accessing sector in read-only mode
    mov al, [N_SECTORS]      ; how many sectors to read
    mov ch, 0                ; cilinder number
    mov cl, 2                ; sector number (counts from 1 (why?))
    mov dh, 0                ; head number
    mov dl, [BOOT_DISK]      ; drive number
    mov bx, KERNEL_LOCATION  ; where to load the sector to
    int 0x13

    jc read_error            ; if error happened, print error message

    mov ah, 0x0              ; set video mode
    mov al, 0x3              ; text - colour
    int 0x10                 ; switches to text mode (also clears the screen)
    
    jmp gdt_setup

read_error:
    mov bx, error_msg        ; load error_msg to bx
    call print_str           ; print it
    jmp $

error_msg:
    db "Error reading disk", 13, 10, 0

BOOT_DISK:
    db 0

N_SECTORS:
    db 80

; ---------------------------------------------------------------------------------------------------------------------------------

CODE_SEG: equ gdt_code - gdt_start
DATA_SEG: equ gdt_data - gdt_start
; TSS_SEG: equ gdt_tss - gdt_start

gdt_setup:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0                         ; cant write directly to cr0, so load it to eax first
    or eax, 1                            ; last bit of cr0 needs to be 1 to allow protected mode
    mov cr0, eax                         ; write back to cr0
    jmp CODE_SEG:start_protected_mode

gdt_start:                               ; needs to be after real mode code
    gdt_null:
        dd 0
        dd 0

    gdt_code:
        dw 0xffff                        ; first 16 bits of the limit
        dw 0                             ; 16 bits +
        db 0                             ;  8 bits = 24 bits of the base
        db 0b10011010                    ; presence, privilege, type and type flags
        db 0b11001111                    ; 4 bits of other flags + other 4 bits of the limit
        db 0                             ; last 8 bits of the base
        
    gdt_data:
        dw 0xffff                        ; 16 bits of the limit
        dw 0
        db 0
        db 0b10010010                    ; presence, privilege, type and type flags
        db 0b11001111                    ; other flags and limit
        db 0

    ; gdt_tss:
    ;     dw tss_end - tss_start - 1       ; limit
    ;     dw tss_start                     ; base low 16 bits
    ;     db equ tss_start >> 16           ; base next 8 bits
    ;     db 0b10001001                    ; presence, privilege, type and type flags
    ;     db 0                             ; flags and limit's high 4 bits
    ;     db equ tss_start >> 24           ; base high 8 bits
        

gdt_end:
    
gdt_descriptor:
    dw gdt_end - gdt_start - 1           ; size of the gdt
    dd gdt_start                         ; start of the gdt

; ---------------------------------------------------------------------------------------------------------------------------------

; tss_start:
;     dd 0                  ; Previous TSS (not used)
;     dd 0                  ; ESP0
;     dw 0                  ; SS0
;     dw 0
;     dd 0                  ; ESP1
;     dw 0                  ; SS1
;     dw 0
;     dd 0                  ; ESP2
;     dw 0                  ; SS2
;     dw 0
;     dd 0                  ; CR3
;     dd 0                  ; EIP
;     dd 0                  ; EFLAGS
;     dd 0                  ; EAX
;     dd 0                  ; ECX
;     dd 0                  ; EDX
;     dd 0                  ; EBX
;     dd 0                  ; ESP
;     dd 0                  ; EBP
;     dd 0                  ; ESI
;     dd 0                  ; EDI
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dw 0
;     dd 0                  ; LDT
;     dw 0                  ; Trap
;     dw 0                  ; IO Map Base
; tss_end:
; 
; ; TSS Segment Selector (GDT index << 3)
; TSS_SEL equ TSS_SEG << 3

; ---------------------------------------------------------------------------------------------------------------------------------

[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax                           ; pointing ds (data segment reg) to data segment 
    mov ss, ax                           ; pointing ss (stack segment reg) to data segment
    mov es, ax                           ; setting es (extra data segment reg) to 0
    mov fs, ax                           ; setting fs (extra extra data segment reg) to 0
    mov gs, ax                           ; setting gs (extra extra extra data segment reg) to 0
    mov ebp, 0x7ffff                     ; setting the base of the stack to 0x80000
    mov esp, ebp                         ; emptying the stack

    ; mov ax, TSS_SEL
    ; ltr ax

    mov dword [0x7ff0], gdt_code         ; saving constant at specific address for reading from kernel later
    
    jmp KERNEL_LOCATION

times 510-($-$$) db 0
dw 0xaa55
