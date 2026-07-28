// =============================================================================
// gui/desktop.cpp -- Desktop environment implementation
// =============================================================================
#include "desktop.h"
#include "compositor.h"
#include "taskbar.h"
#include "apps/terminal.h"
#include "apps/filemanager.h"
#include "apps/settings.h"
#include "libc/stdio.h"

namespace desktop {

void init() {
    compositor::init();

    // Register every built-in app with the launcher. To add a new app:
    //   1. Implement the App interface (see gui/window.h) in apps/yourapp.*
    //   2. Add its factory function here.
    // That's it -- no other file needs to change.
    taskbar::add_launcher("Terminal", create_terminal);
    taskbar::add_launcher("Files", create_filemanager);
    taskbar::add_launcher("Settings", create_settings);

    // Open a terminal by default so the desktop isn't empty on first boot.
    compositor::open_window(create_terminal(), 80, 60, 420, 280);

    kprintf("[desktop] ready\n");
}

} // namespace desktop
