// =============================================================================
// drivers/serial.h -- COM1 serial port driver
// =============================================================================
// Used purely for debug logging. QEMU can redirect the emulated COM1 port to
// your terminal (see the -serial stdio flag in scripts/run.sh), which gives
// us a reliable printf-style debug channel that works even before the
// graphical console is up, and keeps working if the GUI crashes.
// =============================================================================
#pragma once
#include "kernel/types.h"

namespace serial {
    void init();
    void write_char(char c);
    void write(const char* str);
}
