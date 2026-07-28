// =============================================================================
// kernel/memory/heap.h -- Kernel heap allocator
// =============================================================================
// Provides kmalloc()/kfree() for dynamic memory used throughout the kernel
// (window objects, buffers, process structures, etc). Implemented as a
// simple free-list allocator over a fixed heap region carved out of physical
// memory the PMM already tracks.
//
// This is intentionally simple (a linked list of free blocks, first-fit)
// rather than a fancy slab allocator -- easy to read and correct, with a
// clear place to optimize later if profiling ever shows it matters.
// =============================================================================
#pragma once
#include "kernel/types.h"

namespace heap {
    void init();

    void* kmalloc(size_t size);
    void  kfree(void* ptr);
}

// Global operators so normal C++ `new`/`delete` work in the kernel.
void* operator new(size_t size);
void* operator new[](size_t size);
void  operator delete(void* ptr) noexcept;
void  operator delete[](void* ptr) noexcept;
void  operator delete(void* ptr, size_t) noexcept;
void  operator delete[](void* ptr, size_t) noexcept;
