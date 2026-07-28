// =============================================================================
// gui/compositor.h -- Window manager / compositor
// =============================================================================
// The compositor is the heart of the desktop environment. Each frame it:
//   1. Draws the desktop background.
//   2. Draws every window, back-to-front (z-order).
//   3. Draws the taskbar.
//   4. Draws the mouse cursor on top of everything.
//   5. Presents the finished frame to the screen.
//
// It also owns input routing: mouse clicks focus/drag/close windows or
// launch apps from the taskbar, and keyboard input goes to the focused
// window's App.
// =============================================================================
#pragma once
#include "window.h"

constexpr int MAX_WINDOWS = 16;

namespace compositor {
    void init();

    // Creates a window wrapping the given app and brings it to front/focus.
    Window* open_window(App* app, int x, int y, int w, int h);

    // Runs one iteration of: read input -> update state -> draw -> present.
    // Called in a loop from kernel_main's main loop.
    void frame();
}
