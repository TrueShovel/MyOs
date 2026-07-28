// =============================================================================
// drivers/pit.h -- Programmable Interval Timer (PIT) driver
// =============================================================================
// The PIT fires IRQ0 at a configurable frequency. We use it for:
//   - A tick counter other subsystems can read (e.g. to time double-clicks).
//   - sleep_ms(), a busy-wait helper for simple delays.
//   - The scheduler tick, once preemptive multitasking is added (see
//     docs/ARCHITECTURE.md "Process Management" roadmap).
// =============================================================================
#pragma once
#include "kernel/types.h"

namespace pit {
    // Configures the PIT to fire at `frequency_hz` and registers its IRQ0
    // handler.
    void init(uint32_t frequency_hz = 100);

    uint64_t ticks();
    void sleep_ms(uint32_t ms);
}
