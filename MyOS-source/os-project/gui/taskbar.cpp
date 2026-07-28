// =============================================================================
// gui/taskbar.cpp -- Taskbar implementation
// =============================================================================
#include "taskbar.h"
#include "drivers/framebuffer.h"
#include "drivers/pit.h"
#include "libc/stdio.h"

using namespace fb::colors;

namespace {
    LauncherEntry launchers[MAX_LAUNCHERS];
    int launcherCount = 0;

    constexpr int BUTTON_WIDTH = 90;
    constexpr int BUTTON_MARGIN = 8;
}

namespace taskbar {

void add_launcher(const char* label, AppFactory factory) {
    if (launcherCount >= MAX_LAUNCHERS) return;
    launchers[launcherCount++] = { label, factory };
}

void draw(int screen_w, int screen_h) {
    int bar_y = screen_h - TASKBAR_HEIGHT;
    fb::fill_rect(0, bar_y, screen_w, TASKBAR_HEIGHT, SURFACE_ALT);
    fb::fill_rect(0, bar_y, screen_w, 1, BORDER);

    int bx = BUTTON_MARGIN;
    for (int i = 0; i < launcherCount; i++) {
        fb::fill_rect(bx, bar_y + 4, BUTTON_WIDTH, TASKBAR_HEIGHT - 8, SURFACE);
        fb::draw_rect(bx, bar_y + 4, BUTTON_WIDTH, TASKBAR_HEIGHT - 8, BORDER);
        int text_x = bx + (BUTTON_WIDTH - fb::text_width(launchers[i].label)) / 2;
        fb::draw_string(text_x, bar_y + 12, launchers[i].label, TEXT);
        bx += BUTTON_WIDTH + BUTTON_MARGIN;
    }

    // Simple uptime "clock" (seconds since boot) on the right side, since we
    // have no RTC driver yet (see docs/ARCHITECTURE.md roadmap).
    char buf[32];
    uint64_t seconds = pit::ticks() / 100; // pit runs at 100Hz
    uint32_t h = (uint32_t)(seconds / 3600);
    uint32_t m = (uint32_t)((seconds / 60) % 60);
    uint32_t s = (uint32_t)(seconds % 60);

    int idx = 0;
    buf[idx++] = '0' + (h / 10) % 10;
    buf[idx++] = '0' + h % 10;
    buf[idx++] = ':';
    buf[idx++] = '0' + (m / 10) % 10;
    buf[idx++] = '0' + m % 10;
    buf[idx++] = ':';
    buf[idx++] = '0' + (s / 10) % 10;
    buf[idx++] = '0' + s % 10;
    buf[idx] = '\0';

    int clock_w = fb::text_width(buf);
    fb::draw_string(screen_w - clock_w - 16, bar_y + 12, buf, TEXT);
}

const LauncherEntry* hit_test(int x, int y, int screen_h) {
    if (!contains_point(x, y, screen_h)) return nullptr;
    int bar_y = screen_h - TASKBAR_HEIGHT;
    int bx = BUTTON_MARGIN;
    for (int i = 0; i < launcherCount; i++) {
        if (x >= bx && x < bx + BUTTON_WIDTH && y >= bar_y + 4 && y < bar_y + TASKBAR_HEIGHT - 4) {
            return &launchers[i];
        }
        bx += BUTTON_WIDTH + BUTTON_MARGIN;
    }
    return nullptr;
}

bool contains_point(int x, int y, int screen_h) {
    return y >= screen_h - TASKBAR_HEIGHT;
}

} // namespace taskbar
