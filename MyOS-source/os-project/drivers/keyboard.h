// =============================================================================
// drivers/keyboard.h -- PS/2 keyboard driver
// =============================================================================
// Handles IRQ1, translates "scan code set 1" bytes into ASCII, and tracks
// modifier key state (shift, ctrl, caps lock). Consumers (like the terminal
// app) call keyboard::poll_char() to pull the next available character, or
// keyboard::register_listener() to be pushed events directly.
// =============================================================================
#pragma once
#include "kernel/types.h"

namespace keyboard {
    void init();

    // Returns the next queued character, or 0 if none is available.
    // A non-blocking, poll-based API keeps callers (like a simple terminal
    // loop) trivial to write.
    char poll_char();

    // Raw key state, useful for the GUI (e.g. detecting modifier keys for
    // shortcuts like Alt+Tab in the future).
    bool is_shift_down();
    bool is_ctrl_down();
}
