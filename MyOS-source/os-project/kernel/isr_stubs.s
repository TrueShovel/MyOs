; =============================================================================
; isr_stubs.s -- Low-level interrupt entry points
; =============================================================================
; The CPU can only jump to raw addresses when an interrupt fires; it doesn't
; know anything about C++ calling conventions. Each of these tiny stubs:
;   1. Pushes a dummy error code (for exceptions that don't push one).
;   2. Pushes the interrupt number so the common handler knows what fired.
;   3. Jumps to a shared handler that saves the rest of the CPU state and
;      calls into isr_common_handler (kernel/idt.cpp).
;
; Generating 48 near-identical stubs by hand would be error-prone, so we use
; the assembler's macro facility to generate them instead -- one macro
; invocation per interrupt number.
; =============================================================================

section .text
extern isr_common_handler

%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push dword 0        ; fake error code
    push dword %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push dword %1        ; CPU already pushed a real error code
    jmp isr_common_stub
%endmacro

; CPU exceptions 0-31. Exceptions 8, 10-14, 17 push a real error code; the
; rest do not (see Intel SDM Vol. 3A, Chapter 6).
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; Hardware IRQs 0-15, remapped to interrupt vectors 32-47 (see irq.cpp).
%assign i 32
%rep 16
ISR_NOERR i
%assign i i+1
%endrep

isr_common_stub:
    pusha                 ; save edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds
    push eax              ; save data segment

    mov ax, 0x10           ; load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp               ; pass pointer to Registers struct
    call isr_common_handler
    add esp, 4

    pop eax                ; restore original data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8              ; clean up error code + interrupt number
    sti
    iret
