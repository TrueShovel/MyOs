// =============================================================================
// apps/filemanager.h -- File Manager application
// =============================================================================
// A minimal file browser. Ships with an in-memory demo file listing since the
// kernel doesn't have a real filesystem driver yet (see docs/ARCHITECTURE.md,
// "Filesystem" roadmap). Swapping the demo listing for a real one is a
// matter of replacing list_entries()'s data source once a filesystem driver
// exists -- the drawing/click-handling code does not need to change.
// =============================================================================
#pragma once
#include "gui/window.h"

class FileManager : public App {
public:
    void draw(int x, int y, int w, int h) override;
    void on_click(int local_x, int local_y) override;
    const char* title() const override { return "File Manager"; }

private:
    int selected = -1;
};

App* create_filemanager();
