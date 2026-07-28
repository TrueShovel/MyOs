// =============================================================================
// apps/settings.cpp -- Settings application implementation
// =============================================================================
#include "settings.h"
#include "drivers/framebuffer.h"
#include "kernel/memory/pmm.h"

using namespace fb::colors;

void Settings::draw(int x, int y, int w, int h) {
    fb::draw_string(x, y, "System Information", ACCENT);

    char buf[48];
    // total memory
    {
        int pos = 0;
        const char* prefix = "Total RAM: ";
        int j = 0;
        while (prefix[j]) buf[pos++] = prefix[j++];
        uint32_t kb = pmm::total_frames() * 4;
        char digits[16]; int len = 0;
        if (kb == 0) digits[len++] = '0';
        while (kb > 0) { digits[len++] = '0' + (kb % 10); kb /= 10; }
        for (int k = len - 1; k >= 0; k--) buf[pos++] = digits[k];
        const char* suffix = " KiB";
        int s = 0; while (suffix[s]) buf[pos++] = suffix[s++];
        buf[pos] = '\0';
    }
    fb::draw_string(x, y + 16, buf, TEXT);
    fb::draw_string(x, y + 32, "Resolution: framebuffer (see drivers/framebuffer.cpp)", TEXT_DIM);
    fb::draw_string(x, y + 44, "Kernel: MyOS custom C++ kernel v0.1", TEXT_DIM);

    // A toggle checkbox demo, to show how settings would be wired up.
    fb::fill_rect(x, y + 68, 14, 14, showGridLines ? SUCCESS : SURFACE_ALT);
    fb::draw_rect(x, y + 68, 14, 14, BORDER);
    fb::draw_string(x + 22, y + 70, "Show desktop grid lines (demo toggle)", TEXT);
}

void Settings::on_click(int local_x, int local_y) {
    if (local_x >= 0 && local_x < 14 && local_y >= 68 && local_y < 82) {
        showGridLines = !showGridLines;
    }
}

App* create_settings() { return new Settings(); }
