[bits 32]
.text

; void switch_to_task(tcb_t* tcb);
global switch_to_task
switch_to_task:
    push ebx
    push esi
    push edi
    push ebp

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
