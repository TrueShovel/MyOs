// =============================================================================
// gui/compositor.cpp -- Compositor implementation
// =============================================================================
#include "compositor.h"
#include "taskbar.h"
#include "drivers/framebuffer.h"
#include "drivers/mouse.h"
#include "drivers/keyboard.h"
#include "libc/stdio.h"

using namespace fb::colors;

namespace {
    Window* windows[MAX_WINDOWS] = { nullptr };
    int windowCount = 0;
    Window* focused_window = nullptr;

    // Drag state for moving windows by their title bar.
    bool dragging = false;
    Window* drag_window = nullptr;
    int drag_offset_x = 0, drag_offset_y = 0;

    bool prev_left_button = false;

    void bring_to_front(Window* win) {
        int idx = -1;
        for (int i = 0; i < windowCount; i++) if (windows[i] == win) { idx = i; break; }
        if (idx < 0) return;
        for (int i = idx; i < windowCount - 1; i++) windows[i] = windows[i + 1];
        windows[windowCount - 1] = win;
    }

    void handle_mouse() {
        int mx = mouse::x(), my = mouse::y();
        bool left = mouse::left_button();
        bool clicked = left && !prev_left_button; // rising edge = a fresh click

        if (dragging) {
            if (left) {
                drag_window->move_to(mx - drag_offset_x, my - drag_offset_y);
            } else {
                dragging = false;
                drag_window = nullptr;
            }
            prev_left_button = left;
            return;
        }

        if (clicked) {
            // Taskbar clicks take priority.
            const LauncherEntry* entry = taskbar::hit_test(mx, my, fb::height());
            if (entry) {
                App* app = entry->factory();
                compositor::open_window(app, 60 + windowCount * 24, 60 + windowCount * 24, 360, 260);
                prev_left_button = left;
                return;
            }

            // Check windows, front-to-back (topmost first).
            for (int i = windowCount - 1; i >= 0; i--) {
                Window* win = windows[i];
                if (!win->contains(mx, my)) continue;

                bring_to_front(win);
                for (int j = 0; j < windowCount; j++) windows[j]->focused = (windows[j] == win);
                focused_window = win;

                if (win->close_button_contains(mx, my)) {
                    int idx = -1;
                    for (int k = 0; k < windowCount; k++) if (windows[k] == win) { idx = k; break; }
                    delete win->app;
                    delete win;
                    for (int k = idx; k < windowCount - 1; k++) windows[k] = windows[k + 1];
                    windowCount--;
                    if (focused_window == win) focused_window = windowCount ? windows[windowCount - 1] : nullptr;
                } else if (win->titlebar_contains(mx, my)) {
                    dragging = true;
                    drag_window = win;
                    drag_offset_x = mx - win->x;
                    drag_offset_y = my - win->y;
                } else {
                    int local_x = mx - (win->x + 8);
                    int local_y = my - (win->y + TITLEBAR_HEIGHT + 8);
                    win->app->on_click(local_x, local_y);
                }
                break;
            }
        }

        prev_left_button = left;
    }

    void handle_keyboard() {
        char c;
        while ((c = keyboard::poll_char()) != 0) {
            if (focused_window) focused_window->app->on_key(c);
        }
    }

    void draw_desktop_background() {
        fb::fill_rect(0, 0, fb::width(), fb::height(), BACKGROUND);
        for (uint32_t x = 0; x < fb::width(); x += 64) {
            fb::fill_rect(x, 0, 1, fb::height() - TASKBAR_HEIGHT, 0x232733);
        }
        fb::draw_string(16, 16, "MyOS", TEXT_DIM);
    }

    void draw_cursor() {
        int mx = mouse::x(), my = mouse::y();
        for (int i = 0; i < 10; i++) {
            fb::draw_line(mx, my, mx, my + i, WHITE);
        }
        for (int i = 0; i < 6; i++) {
            fb::draw_line(mx, my + i, mx + i, my + 6, WHITE);
        }
        fb::draw_rect(mx, my, 1, 1, BLACK);
    }
}

namespace compositor {

void init() {
    kprintf("[compositor] initialized\n");
}

Window* open_window(App* app, int x, int y, int w, int h) {
    if (windowCount >= MAX_WINDOWS) {
        kprintf("[compositor] window limit reached, ignoring open request\n");
        return nullptr;
    }
    Window* win = new Window(app, x, y, w, h);
    windows[windowCount++] = win;

    for (int i = 0; i < windowCount; i++) windows[i]->focused = (windows[i] == win);
    focused_window = win;

    return win;
}

void frame() {
    handle_mouse();
    handle_keyboard();

    draw_desktop_background();
    for (int i = 0; i < windowCount; i++) windows[i]->draw();
    taskbar::draw(fb::width(), fb::height());
    draw_cursor();

    fb::present();
}

} // namespace compositor
