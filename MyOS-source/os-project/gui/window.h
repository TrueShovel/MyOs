// =============================================================================
// gui/window.h -- A single GUI window
// =============================================================================
// A Window is a movable, titled rectangle that owns a content area. Each
// application (terminal, file manager, settings) implements the App
// interface below and is wrapped in a Window by the compositor.
//
// This class deliberately knows nothing about *how* it gets drawn to the
// screen -- it just draws itself into the framebuffer at its own position.
// The compositor (gui/compositor.h) owns the list of windows, z-ordering,
// and input routing.
// =============================================================================
#pragma once
#include "kernel/types.h"

constexpr int TITLEBAR_HEIGHT = 24;

// Interface every application implements to live inside a Window.
// This is the "application framework" the OS exposes for future programs:
// to add a new app, implement this interface and register it with the
// desktop (see apps/terminal.h for a worked example).
class App {
public:
    virtual ~App() {}

    // Called once per frame to draw the app's content. (x,y) is the
    // top-left of the content area (already excludes the title bar),
    // and (w,h) is the available size.
    virtual void draw(int x, int y, int w, int h) = 0;

    // Called when a printable character is typed while this app's window
    // has focus.
    virtual void on_key(char c) {}

    // Called on left-click within the content area, with coordinates
    // relative to the content area's top-left corner.
    virtual void on_click(int local_x, int local_y) {}

    virtual const char* title() const = 0;
};

class Window {
public:
    Window(App* app, int x, int y, int w, int h);

    void draw();
    void move_to(int x, int y);

    int x, y, w, h;
    bool focused = false;
    bool minimized = false;
    App* app;

    bool contains(int px, int py) const;
    bool titlebar_contains(int px, int py) const;
    bool close_button_contains(int px, int py) const;
};
