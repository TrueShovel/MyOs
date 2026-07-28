// =============================================================================
// kernel/gdt.cpp -- GDT implementation
// =============================================================================
#include "gdt.h"
#include "types.h"

namespace {

// One 8-byte GDT entry, packed to match the exact hardware layout the CPU
// expects (see Intel SDM Vol. 3A, Section 3.4.5).
struct __attribute__((packed)) GdtEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
};

struct __attribute__((packed)) GdtPointer {
    uint16_t limit;
    uint32_t base;
};

constexpr int GDT_ENTRIES = 5;
GdtEntry gdt_table[GDT_ENTRIES];
GdtPointer gdt_ptr;

void set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_table[index].base_low    = base & 0xFFFF;
    gdt_table[index].base_middle = (base >> 16) & 0xFF;
    gdt_table[index].base_high   = (base >> 24) & 0xFF;
    gdt_table[index].limit_low   = limit & 0xFFFF;
    gdt_table[index].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_table[index].access      = access;
}

extern "C" void gdt_flush(uint32_t gdt_ptr_addr);

} // namespace

namespace gdt {

void init() {
    gdt_ptr.limit = sizeof(GdtEntry) * GDT_ENTRIES - 1;
    gdt_ptr.base  = (uint32_t)&gdt_table;

    set_entry(0, 0, 0, 0, 0);                       // null descriptor (required)
    set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);         // kernel code segment
    set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);         // kernel data segment
    set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);         // user code segment (future use)
    set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);         // user data segment (future use)

    gdt_flush((uint32_t)&gdt_ptr);
}

} // namespace gdt

// Loads the GDT register and reloads every segment register. Must be in
// assembly because you cannot reload CS from C++ (requires a far jump).
extern "C" void gdt_flush(uint32_t gdt_ptr_addr) {
    asm volatile (
        "lgdt (%0)\n"
        "mov $0x10, %%ax\n"   // 0x10 = kernel data segment selector
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $.flush\n" // 0x08 = kernel code segment selector
        ".flush:\n"
        :
        : "r"(gdt_ptr_addr)
        : "eax"
    );
}
