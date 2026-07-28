; =============================================================================
; boot.s -- Boot entry point
; =============================================================================
; This is the VERY FIRST code that runs. GRUB (acting as our bootloader) loads
; this file according to the Multiboot2 specification and jumps to _start.
;
; Responsibilities of this file:
;   1. Provide a Multiboot2 header so GRUB recognizes this as a bootable kernel.
;   2. Set up a small stack (C++ code cannot run without one).
;   3. Save the Multiboot info pointer GRUB gives us (memory map, framebuffer
;      info, etc.) so the C++ kernel can use it.
;   4. Call the global constructors for any C++ static objects, then jump into
;      kernel_main() (implemented in kernel/kernel_main.cpp).
;
; Why assembly here and not C++?  The CPU starts in a state where there is no
; stack and no C++ runtime yet, so a tiny hand-written entry point is required
; before we can safely run C++ code.
; =============================================================================

section .multiboot2
align 8
multiboot_header_start:
    dd 0xE85250D6                ; Multiboot2 magic number
    dd 0                         ; Architecture 0 = i386 protected mode
    dd multiboot_header_end - multiboot_header_start
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header_start)) ; checksum

    ; --- Framebuffer request tag ---
    ; Ask GRUB to set up a linear graphics framebuffer for us before handing
    ; control to the kernel. This lets us do real graphics without touching
    ; VGA registers ourselves (works on real hardware via VBE/UEFI GOP too).
    align 8
    dw 5                          ; type = framebuffer
    dw 0                          ; flags
    dd 20                         ; size of this tag
    dd 1024                       ; preferred width
    dd 768                        ; preferred height
    dd 32                         ; preferred bits per pixel

    ; --- End tag ---
    align 8
    dw 0
    dw 0
    dd 8
multiboot_header_end:

; -----------------------------------------------------------------------------
; .bss: reserve space for our initial kernel stack.
; -----------------------------------------------------------------------------
section .bss
align 16
stack_bottom:
    resb 65536                    ; 64 KiB kernel stack
stack_top:

; -----------------------------------------------------------------------------
; .text: the actual entry code
; -----------------------------------------------------------------------------
section .text
global _start
extern kernel_main
extern init_constructors        ; runs C++ global constructors (kernel/crt.cpp)

_start:
    ; GRUB leaves us in 32-bit protected mode with:
    ;   EAX = multiboot2 magic value (0x36d76289)
    ;   EBX = physical address of the multiboot info structure
    cli                          ; disable interrupts until IDT is ready
    mov esp, stack_top           ; set up the stack pointer
    mov ebp, esp

    push ebx                     ; save multiboot info pointer
    push eax                     ; save multiboot magic

    call init_constructors       ; run C++ static/global constructors

    pop eax
    pop ebx
    push ebx                     ; pass multiboot info ptr as 2nd arg
    push eax                     ; pass multiboot magic as 1st arg
    call kernel_main             ; jump into C++ land -- never returns

.hang:
    cli
    hlt
    jmp .hang
