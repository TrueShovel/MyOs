// =============================================================================
// apps/settings.h -- Settings application
// =============================================================================
// Shows live system information and a couple of toggleable demo settings.
// This is a good template for adding real settings later (display, network,
// etc.) as those subsystems are built -- see docs/ADDING_APPS.md.
// =============================================================================
#pragma once
#include "gui/window.h"

class Settings : public App {
public:
    void draw(int x, int y, int w, int h) override;
    void on_click(int local_x, int local_y) override;
    const char* title() const override { return "Settings"; }

private:
    bool showGridLines = false;
};

App* create_settings();
