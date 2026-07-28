// =============================================================================
// gui/window.cpp -- Window implementation
// =============================================================================
#include "window.h"
#include "drivers/framebuffer.h"

using namespace fb::colors;

Window::Window(App* app_, int x_, int y_, int w_, int h_)
    : x(x_), y(y_), w(w_), h(h_), app(app_) {}

void Window::move_to(int nx, int ny) {
    x = nx;
    y = ny;
}

bool Window::contains(int px, int py) const {
    return px >= x && px < x + w && py >= y && py < y + h;
}

bool Window::titlebar_contains(int px, int py) const {
    return px >= x && px < x + w && py >= y && py < y + TITLEBAR_HEIGHT;
}

bool Window::close_button_contains(int px, int py) const {
    int bx = x + w - 20, by = y + 4;
    return px >= bx && px < bx + 14 && py >= by && py < by + 14;
}

void Window::draw() {
    if (minimized) return;

    // Drop shadow for depth.
    fb::fill_rect(x + 4, y + 4, w, h, 0x0D0F13);

    // Window body.
    fb::fill_rect(x, y, w, h, SURFACE);
    fb::draw_rect(x, y, w, h, focused ? ACCENT : BORDER);

    // Title bar.
    Color titlebar_color = focused ? ACCENT_DIM : SURFACE_ALT;
    fb::fill_rect(x, y, w, TITLEBAR_HEIGHT, titlebar_color);
    fb::draw_string(x + 8, y + 8, app->title(), TEXT);

    // Close button.
    int bx = x + w - 20, by = y + 4;
    fb::fill_rect(bx, by, 14, 14, DANGER);
    fb::draw_line(bx + 3, by + 3, bx + 10, by + 10, WHITE);
    fb::draw_line(bx + 10, by + 3, bx + 3, by + 10, WHITE);

    // Content area, clipped visually by drawing a background first;
    // (true pixel clipping of app draw calls is a documented roadmap item).
    int content_y = y + TITLEBAR_HEIGHT;
    int content_h = h - TITLEBAR_HEIGHT;
    fb::fill_rect(x + 1, content_y, w - 2, content_h - 1, BACKGROUND);
    app->draw(x + 8, content_y + 8, w - 16, content_h - 16);
}
