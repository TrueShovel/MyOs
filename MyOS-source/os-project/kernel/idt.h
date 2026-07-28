// =============================================================================
// kernel/idt.h -- Interrupt Descriptor Table & interrupt dispatch
// =============================================================================
// The IDT tells the CPU where to jump when a CPU exception (divide-by-zero,
// page fault, ...) or a hardware interrupt (keyboard, timer, ...) occurs.
// This is what makes drivers "event driven" instead of having to poll
// hardware constantly.
//
// Layout of interrupt numbers we use:
//   0-31   CPU exceptions (fixed by the x86 architecture)
//   32-47  Hardware IRQs 0-15, remapped here from their BIOS defaults of
//          8-15/0-7 to avoid colliding with CPU exceptions (see irq.cpp)
//   48+    free for software interrupts / future syscalls
// =============================================================================
#pragma once
#include "types.h"

// The register state pushed onto the stack by our ISR stubs before calling
// into C++. Handlers receive a pointer to this so they can inspect (and, for
// syscalls, modify) CPU state.
struct Registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp_unused, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

using InterruptHandler = void(*)(Registers*);

namespace idt {
    void init();

    // Registers a C++ handler function for a given interrupt/IRQ number.
    // This is how drivers hook themselves up -- e.g. the keyboard driver
    // calls register_handler(33, keyboard::handle_irq).
    void register_handler(uint8_t num, InterruptHandler handler);
}
