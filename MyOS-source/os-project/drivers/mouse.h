// =============================================================================
// drivers/mouse.h -- PS/2 mouse driver
// =============================================================================
// Enables the PS/2 auxiliary device (the mouse), handles IRQ12, and decodes
// the standard 3-byte PS/2 mouse packet into an absolute cursor position
// clamped to the screen bounds. The GUI compositor reads this every frame to
// draw and hit-test the cursor.
// =============================================================================
#pragma once
#include "kernel/types.h"

namespace mouse {
    void init(int screen_width, int screen_height);

    int  x();
    int  y();
    bool left_button();
    bool right_button();
}
