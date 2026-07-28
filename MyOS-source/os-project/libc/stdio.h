// =============================================================================
// libc/stdio.h -- Minimal formatted printing for kernel use
// =============================================================================
// A tiny printf() work-alike. Supports %d %u %x %p %s %c %% only -- enough
// for kernel debug logs. Output goes to the serial port (see drivers/serial)
// so you can see kernel logs on the host terminal running QEMU.
// =============================================================================
#pragma once

// Prints a formatted string to the serial debug console.
void kprintf(const char* fmt, ...);
