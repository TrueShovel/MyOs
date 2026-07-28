// =============================================================================
// apps/terminal.h -- Terminal application
// =============================================================================
// A simple command-line app living inside a GUI window. Demonstrates how to
// build an App (see gui/window.h): it buffers typed characters into a line,
// executes a small set of built-in commands on Enter, and scrolls a history
// of output lines. This is the reference example to copy when writing new
// applications -- see docs/ADDING_APPS.md.
// =============================================================================
#pragma once
#include "gui/window.h"

class Terminal : public App {
public:
    Terminal();

    void draw(int x, int y, int w, int h) override;
    void on_key(char c) override;
    const char* title() const override { return "Terminal"; }

private:
    static constexpr int MAX_LINES = 128;
    static constexpr int LINE_LEN = 64;

    char history[MAX_LINES][LINE_LEN];
    int historyCount = 0;

    char inputLine[LINE_LEN];
    int inputLen = 0;

    void push_line(const char* text);
    void run_command();
};

// Factory function registered with the taskbar launcher (see gui/taskbar.h).
App* create_terminal();
