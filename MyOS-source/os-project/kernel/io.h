// =============================================================================
// io.h -- x86 port I/O primitives
// =============================================================================
// Almost every hardware driver in this OS (VGA, keyboard, mouse, PIT, serial)
// talks to its device through "I/O ports" -- a separate address space the x86
// CPU exposes via the IN/OUT instructions. These tiny wrapper functions are
// the foundation that every driver in drivers/ builds on.
//
// Keeping this in one header means:
//   - There's exactly one place that contains inline assembly for port I/O.
//   - Any new driver just does `#include "kernel/io.h"` and calls outb/inb.
// =============================================================================
#pragma once
#include "types.h"

// Write a single byte to an I/O port.
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read a single byte from an I/O port.
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a 16-bit word to an I/O port.
static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// A tiny delay used after certain PIC/PS2 commands that need the hardware a
// moment to catch up. Writing to an unused port (0x80, POST diagnostic port)
// takes roughly 1 microsecond on real hardware and is the classic trick.
static inline void io_wait() {
    outb(0x80, 0);
}
