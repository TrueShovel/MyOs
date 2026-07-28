// =============================================================================
// gui/desktop.h -- Desktop environment bootstrap
// =============================================================================
// The "desktop environment" is really just: initialize the compositor,
// register the built-in apps with the taskbar launcher, and open a starting
// window. kernel_main() calls desktop::init() once, then loops calling
// compositor::frame() forever.
// =============================================================================
#pragma once

namespace desktop {
    void init();
}
