// =============================================================================
// kernel/memory/pmm.h -- Physical Memory Manager
// =============================================================================
// Tracks which 4 KiB physical page frames are free or in use, using one bit
// per frame ("bitmap allocator" -- the simplest correct design). The heap
// allocator (kernel/memory/heap.cpp) asks the PMM for pages when it needs to
// grow, and drivers can also request raw physical pages directly (e.g. for
// DMA buffers) via alloc_frame/free_frame.
//
// This is a classic, well-documented design used by most teaching OSes
// (see OSDev.org's "Page Frame Allocation" article) because it's simple to
// reason about and easy to extend later into a buddy allocator if needed.
// =============================================================================
#pragma once
#include "kernel/types.h"

namespace pmm {
    // Initializes the allocator using the memory map already parsed by
    // multiboot::parse(). Marks the kernel image itself as reserved so it's
    // never handed out.
    void init();

    // Allocates one free 4 KiB physical page frame. Returns its physical
    // address, or 0 if out of memory.
    uintptr_t alloc_frame();

    // Frees a previously allocated frame.
    void free_frame(uintptr_t addr);

    uint32_t total_frames();
    uint32_t free_frames();
}
