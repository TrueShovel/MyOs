// =============================================================================
// kernel/multiboot.cpp -- Multiboot2 info parsing implementation
// =============================================================================
#include "multiboot.h"
#include "libc/stdio.h"

namespace {

struct __attribute__((packed)) TagHeader {
    uint32_t type;
    uint32_t size;
};

constexpr uint32_t TAG_MEMORY_MAP = 6;
constexpr uint32_t TAG_FRAMEBUFFER = 8;
constexpr uint32_t TAG_END = 0;

MemoryRegion regions[64];
int regionCount = 0;
FramebufferInfo fbInfo = {};

} // namespace

namespace multiboot {

void parse(uint32_t mb_info_addr) {
    uint32_t total_size = *(uint32_t*)mb_info_addr;
    uint8_t* ptr = (uint8_t*)(mb_info_addr + 8); // skip total_size + reserved
    uint8_t* end = (uint8_t*)(mb_info_addr + total_size);

    while (ptr < end) {
        TagHeader* tag = (TagHeader*)ptr;
        if (tag->type == TAG_END) break;

        if (tag->type == TAG_MEMORY_MAP) {
            struct __attribute__((packed)) MmapEntry {
                uint64_t base, length;
                uint32_t type, reserved;
            };
            uint32_t entry_size = *(uint32_t*)(ptr + 8);
            uint8_t* entry = ptr + 16;
            uint8_t* mmap_end = ptr + tag->size;
            while (entry < mmap_end && regionCount < 64) {
                MmapEntry* e = (MmapEntry*)entry;
                if (e->type == 1 /* available RAM */) {
                    regions[regionCount].base = e->base;
                    regions[regionCount].length = e->length;
                    regionCount++;
                }
                entry += entry_size;
            }
        } else if (tag->type == TAG_FRAMEBUFFER) {
            struct __attribute__((packed)) FbTag {
                TagHeader header;
                uint64_t addr;
                uint32_t pitch;
                uint32_t width;
                uint32_t height;
                uint8_t  bpp;
                uint8_t  type;
                uint16_t reserved;
            };
            FbTag* fb = (FbTag*)ptr;
            fbInfo.address = fb->addr;
            fbInfo.pitch = fb->pitch;
            fbInfo.width = fb->width;
            fbInfo.height = fb->height;
            fbInfo.bpp = fb->bpp;
            fbInfo.valid = true;
        }

        // Tags are 8-byte aligned.
        ptr += (tag->size + 7) & ~7;
    }

    kprintf("[multiboot] found %d usable memory regions\n", regionCount);
    if (fbInfo.valid) {
        kprintf("[multiboot] framebuffer %ux%u @ %d bpp, addr=%p\n",
                fbInfo.width, fbInfo.height, fbInfo.bpp, (void*)(uint32_t)fbInfo.address);
    } else {
        kprintf("[multiboot] no framebuffer tag found!\n");
    }
}

uint64_t highest_usable_address() {
    uint64_t highest = 0;
    for (int i = 0; i < regionCount; i++) {
        uint64_t top = regions[i].base + regions[i].length;
        if (top > highest) highest = top;
    }
    return highest;
}

int region_count() { return regionCount; }
const MemoryRegion& region(int index) { return regions[index]; }
const FramebufferInfo& framebuffer() { return fbInfo; }

} // namespace multiboot
