// =============================================================================
// drivers/pit.cpp -- PIT timer driver implementation
// =============================================================================
#include "pit.h"
#include "kernel/io.h"
#include "kernel/idt.h"
#include "libc/stdio.h"

namespace {
    constexpr uint16_t PIT_CHANNEL0 = 0x40;
    constexpr uint16_t PIT_COMMAND  = 0x43;
    constexpr uint32_t PIT_BASE_FREQ = 1193182;

    volatile uint64_t tick_count = 0;
    uint32_t configuredHz = 100;

    void irq0_handler(Registers*) {
        tick_count++;
    }
}

namespace pit {

void init(uint32_t frequency_hz) {
    configuredHz = frequency_hz;
    uint32_t divisor = PIT_BASE_FREQ / frequency_hz;

    outb(PIT_COMMAND, 0x36);                    // channel 0, lobyte/hibyte, square wave
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    idt::register_handler(32, irq0_handler);     // IRQ0 -> vector 32
    kprintf("[pit] configured for %u Hz\n", frequency_hz);
}

uint64_t ticks() { return tick_count; }

void sleep_ms(uint32_t ms) {
    uint64_t target = tick_count + (ms * configuredHz) / 1000;
    while (tick_count < target) {
        asm volatile ("sti; hlt"); // sleep until next interrupt
    }
}

} // namespace pit
