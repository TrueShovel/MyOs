// =============================================================================
// kernel/memory/pmm.cpp -- Physical Memory Manager implementation
// =============================================================================
#include "pmm.h"
#include "kernel/multiboot.h"
#include "libc/string.h"
#include "libc/stdio.h"

extern "C" uint32_t kernel_end_marker; // defined by boot/linker.ld

namespace {
    constexpr uint32_t PAGE_SIZE = 4096;
    constexpr uint32_t MAX_FRAMES = 1024 * 1024; // supports up to 4 GiB of RAM
    constexpr uint32_t BITMAP_WORDS = MAX_FRAMES / 32;

    uint32_t bitmap[BITMAP_WORDS];
    uint32_t totalFrames = 0;
    uint32_t freeFramesCount = 0;

    inline void set_bit(uint32_t frame)   { bitmap[frame / 32] |= (1u << (frame % 32)); }
    inline void clear_bit(uint32_t frame) { bitmap[frame / 32] &= ~(1u << (frame % 32)); }
    inline bool test_bit(uint32_t frame)  { return bitmap[frame / 32] & (1u << (frame % 32)); }
}

namespace pmm {

void init() {
    // Start with everything marked used; we'll clear bits for frames that
    // the memory map reports as available.
    memset(bitmap, 0xFF, sizeof(bitmap));

    uint64_t highest = multiboot::highest_usable_address();
    totalFrames = (uint32_t)(highest / PAGE_SIZE);
    if (totalFrames > MAX_FRAMES) totalFrames = MAX_FRAMES;

    for (int i = 0; i < multiboot::region_count(); i++) {
        const MemoryRegion& r = multiboot::region(i);
        uint64_t start_frame = r.base / PAGE_SIZE;
        uint64_t end_frame = (r.base + r.length) / PAGE_SIZE;
        for (uint64_t f = start_frame; f < end_frame && f < MAX_FRAMES; f++) {
            clear_bit((uint32_t)f);
            freeFramesCount++;
        }
    }

    // Reserve the first 1 MiB (BIOS/legacy structures) and the kernel image
    // itself so we never hand those frames out.
    uint32_t kernel_end_frame = ((uint32_t)&kernel_end_marker) / PAGE_SIZE + 1;
    for (uint32_t f = 0; f < kernel_end_frame; f++) {
        if (!test_bit(f)) { set_bit(f); freeFramesCount--; }
    }

    kprintf("[pmm] %u total frames, %u free (%u KiB)\n",
            totalFrames, freeFramesCount, freeFramesCount * 4);
}

uintptr_t alloc_frame() {
    for (uint32_t i = 0; i < totalFrames; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            freeFramesCount--;
            return i * PAGE_SIZE;
        }
    }
    return 0; // out of memory
}

void free_frame(uintptr_t addr) {
    uint32_t frame = addr / PAGE_SIZE;
    if (test_bit(frame)) {
        clear_bit(frame);
        freeFramesCount++;
    }
}

uint32_t total_frames() { return totalFrames; }
uint32_t free_frames() { return freeFramesCount; }

} // namespace pmm
