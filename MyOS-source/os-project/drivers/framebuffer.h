// =============================================================================
// drivers/framebuffer.h -- Linear framebuffer graphics driver
// =============================================================================
// Draws directly into the linear framebuffer GRUB set up for us (see the
// multiboot2 framebuffer tag in boot/boot.s and kernel/multiboot.cpp). This
// is the lowest layer of the graphics stack; everything in gui/ is built on
// top of the handful of primitives here (set_pixel, fill_rect, blit, etc).
//
// We render each frame into an off-screen back buffer and copy it to the
// real framebuffer in one shot (present()) to avoid visible flicker/tearing
// when the compositor redraws the whole screen every frame.
// =============================================================================
#pragma once
#include "kernel/types.h"

// 0xRRGGBB color, converted to the framebuffer's native pixel format
// internally. Keeping colors in one universal format makes UI code portable
// across different bit depths.
using Color = uint32_t;

namespace fb {
    bool init();

    uint32_t width();
    uint32_t height();

    void set_pixel(int x, int y, Color color);
    void fill_rect(int x, int y, int w, int h, Color color);
    void draw_rect(int x, int y, int w, int h, Color color); // outline only
    void draw_line(int x0, int y0, int x1, int y1, Color color);
    void draw_char(int x, int y, char c, Color color);
    void draw_string(int x, int y, const char* str, Color color);
    int  text_width(const char* str); // width in pixels for the given string

    // Copies the back buffer to the real framebuffer. Call once per frame,
    // after all drawing is done.
    void present();

    // Common palette used throughout the GUI (see docs/ARCHITECTURE.md for
    // the design rationale).
    namespace colors {
        constexpr Color BACKGROUND   = 0x1E2129;
        constexpr Color SURFACE      = 0x2A2E37;
        constexpr Color SURFACE_ALT  = 0x343945;
        constexpr Color ACCENT       = 0x4C8DFF;
        constexpr Color ACCENT_DIM   = 0x30506B;
        constexpr Color TEXT         = 0xEAECEF;
        constexpr Color TEXT_DIM     = 0x9AA0AC;
        constexpr Color BORDER       = 0x454B58;
        constexpr Color WHITE        = 0xFFFFFF;
        constexpr Color BLACK        = 0x000000;
        constexpr Color SUCCESS      = 0x4CD787;
        constexpr Color WARNING      = 0xFFB454;
        constexpr Color DANGER       = 0xFF5C5C;
    }
}
