// =============================================================================
// kernel/multiboot.h -- Multiboot2 info structure parsing
// =============================================================================
// GRUB passes us a pointer to a tagged list of information structures: the
// memory map, framebuffer details, bootloader name, etc. This module walks
// that list once at boot and extracts the two things the rest of the kernel
// cares about: available RAM regions (for the PMM) and framebuffer info
// (for the graphics driver).
// =============================================================================
#pragma once
#include "types.h"

struct MemoryRegion {
    uint64_t base;
    uint64_t length;
};

struct FramebufferInfo {
    uint64_t address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t  bpp;
    bool     valid;
};

namespace multiboot {
    // Parses the multiboot2 info structure at `mb_info_addr`. Call once, early
    // in kernel_main, before memory or graphics are initialized.
    void parse(uint32_t mb_info_addr);

    // Returns the highest usable physical address found in the memory map.
    // Used by the PMM to size its allocation bitmap.
    uint64_t highest_usable_address();

    int region_count();
    const MemoryRegion& region(int index);

    const FramebufferInfo& framebuffer();
}
