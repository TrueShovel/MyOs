// =============================================================================
// gui/taskbar.h -- Taskbar / dock
// =============================================================================
// Draws the bar along the bottom of the screen: app launcher buttons on the
// left, and a live clock (driven by the PIT tick count) on the right.
// Clicking a launcher button asks the compositor to open a new instance of
// that app -- see LauncherEntry::factory.
// =============================================================================
#pragma once
#include "window.h"

constexpr int TASKBAR_HEIGHT = 32;
constexpr int MAX_LAUNCHERS = 8;

// Function pointer type: every launchable app provides one of these to
// construct a fresh instance of itself. Keeping this as a plain function
// pointer (rather than std::function) avoids needing any STL in the kernel.
using AppFactory = App* (*)();

struct LauncherEntry {
    const char* label;
    AppFactory factory;
};

namespace taskbar {
    // Registers an app in the launcher. Call during desktop::init() for each
    // built-in app -- see apps/*.h for the factory functions to pass in.
    void add_launcher(const char* label, AppFactory factory);

    void draw(int screen_w, int screen_h);

    // Returns the launcher clicked at (x,y), or nullptr if none was hit.
    const LauncherEntry* hit_test(int x, int y, int screen_h);

    bool contains_point(int x, int y, int screen_h);
}
