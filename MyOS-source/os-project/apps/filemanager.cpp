// =============================================================================
// apps/filemanager.cpp -- File Manager implementation
// =============================================================================
#include "filemanager.h"
#include "drivers/framebuffer.h"

using namespace fb::colors;

namespace {
    struct DemoEntry { const char* name; bool is_dir; };
    const DemoEntry demoEntries[] = {
        { "Documents", true },
        { "Downloads", true },
        { "Pictures",  true },
        { "readme.txt", false },
        { "notes.txt",  false },
    };
    constexpr int DEMO_COUNT = sizeof(demoEntries) / sizeof(demoEntries[0]);
}

void FileManager::draw(int x, int y, int w, int h) {
    int rowHeight = 20;
    for (int i = 0; i < DEMO_COUNT; i++) {
        int rowY = y + i * rowHeight;
        if (i == selected) {
            fb::fill_rect(x - 4, rowY - 2, w + 8, rowHeight, ACCENT_DIM);
        }
        Color icon_color = demoEntries[i].is_dir ? ACCENT : TEXT_DIM;
        fb::fill_rect(x, rowY + 2, 10, 10, icon_color);
        fb::draw_string(x + 18, rowY, demoEntries[i].name, TEXT);
    }
}

void FileManager::on_click(int local_x, int local_y) {
    int rowHeight = 20;
    int idx = local_y / rowHeight;
    if (idx >= 0 && idx < DEMO_COUNT) selected = idx;
}

App* create_filemanager() { return new FileManager(); }
