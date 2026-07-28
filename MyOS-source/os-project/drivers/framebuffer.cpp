// =============================================================================
// drivers/framebuffer.cpp -- Framebuffer driver implementation
// =============================================================================
#include "framebuffer.h"
#include "kernel/multiboot.h"
#include "kernel/memory/heap.h"
#include "gui/font8x8.h"
#include "libc/stdio.h"
#include "libc/string.h"

namespace {
    uint8_t* real_fb = nullptr;   // the actual hardware framebuffer
    uint32_t* back_buffer = nullptr; // our off-screen drawing surface

    uint32_t fbWidth = 0, fbHeight = 0, fbPitch = 0;
    uint8_t  fbBpp = 32;

    inline bool in_bounds(int x, int y) {
        return x >= 0 && y >= 0 && x < (int)fbWidth && y < (int)fbHeight;
    }

    const uint8_t* find_glyph_rows(char c) {
        for (int i = 0; i < FONT_GLYPH_COUNT; i++) {
            if (font8x8[i].ch == c) return font8x8[i].rows;
        }
        return nullptr; // unknown character -- draw nothing
    }
}

namespace fb {

bool init() {
    const FramebufferInfo& info = multiboot::framebuffer();
    if (!info.valid || info.bpp != 32) {
        kprintf("[fb] no usable 32bpp linear framebuffer available\n");
        return false;
    }

    real_fb  = (uint8_t*)(uintptr_t)info.address;
    fbWidth  = info.width;
    fbHeight = info.height;
    fbPitch  = info.pitch;
    fbBpp    = info.bpp;

    back_buffer = (uint32_t*)heap::kmalloc(fbWidth * fbHeight * 4);
    if (!back_buffer) {
        kprintf("[fb] failed to allocate back buffer\n");
        return false;
    }

    kprintf("[fb] initialized %ux%u @ 32bpp, back buffer allocated\n", fbWidth, fbHeight);
    return true;
}

uint32_t width()  { return fbWidth; }
uint32_t height() { return fbHeight; }

void set_pixel(int x, int y, Color color) {
    if (!in_bounds(x, y)) return;
    back_buffer[y * fbWidth + x] = color;
}

void fill_rect(int x, int y, int w, int h, Color color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= (int)fbHeight) continue;
        int x0 = x, x1 = x + w;
        if (x0 < 0) x0 = 0;
        if (x1 > (int)fbWidth) x1 = fbWidth;
        for (int i = x0; i < x1; i++) back_buffer[j * fbWidth + i] = color;
    }
}

void draw_rect(int x, int y, int w, int h, Color color) {
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h - 1, w, 1, color);
    fill_rect(x, y, 1, h, color);
    fill_rect(x + w - 1, y, 1, h, color);
}

void draw_line(int x0, int y0, int x1, int y1, Color color) {
    // Bresenham's line algorithm -- the standard approach for pixel-exact
    // lines without floating point.
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void draw_char(int x, int y, char c, Color color) {
    if (c >= 'a' && c <= 'z') c -= 32; // font only defines uppercase
    const uint8_t* rows = find_glyph_rows(c);
    if (!rows) return;

    for (int row = 0; row < 8; row++) {
        uint8_t bits = rows[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                set_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_string(int x, int y, const char* str, Color color) {
    int cx = x;
    while (*str) {
        if (*str == '\n') { cx = x; y += 10; str++; continue; }
        draw_char(cx, y, *str, color);
        cx += 9; // 8px glyph + 1px spacing
        str++;
    }
}

int text_width(const char* str) {
    return (int)strlen(str) * 9;
}

void present() {
    // Copy the back buffer into the real framebuffer, row by row, honoring
    // the hardware's pitch (which may include padding bytes per scanline).
    for (uint32_t y = 0; y < fbHeight; y++) {
        memcpy(real_fb + y * fbPitch, back_buffer + y * fbWidth, fbWidth * 4);
    }
}

} // namespace fb
