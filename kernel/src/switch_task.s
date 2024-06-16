[bits 32]
.text

extern curr_proc

global switch_to
switch_to:
    push ebx
    push esi
    push edi
    push ebp

    mov edi, [curr_proc+4+4+4+4+4]
    mov [edi+4+4+4+4+4+4+4], esp

    mov esi, [esp+(4+1)*4]
    mov [curr_proc], esi

    mov esp, [esi]

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret
