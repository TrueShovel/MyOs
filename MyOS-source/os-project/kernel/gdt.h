// =============================================================================
// kernel/gdt.h -- Global Descriptor Table
// =============================================================================
// x86 protected mode requires a GDT describing memory "segments" before the
// CPU will run any code at all. GRUB sets up a temporary one to get us into
// protected mode, but we replace it with our own so we control it going
// forward (required, e.g., before we can set up user-mode segments later).
//
// We use the "flat model": one segment for code, one for data, each spanning
// the entire 4 GiB address space. All real memory protection in this OS comes
// from paging, not segmentation (segmentation is a legacy x86 feature that
// modern OSes mostly use in this pass-through way).
// =============================================================================
#pragma once

namespace gdt {
    // Sets up the GDT and reloads the segment registers. Must be called
    // before interrupts are enabled.
    void init();
}
