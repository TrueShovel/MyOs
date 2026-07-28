// =============================================================================
// kernel/kernel_main.cpp -- Kernel entry point
// =============================================================================
// This is where control lands after boot/boot.s finishes its minimal setup.
// kernel_main() brings up every subsystem in the correct dependency order,
// then hands off to the desktop's main loop. If you're trying to understand
// how the whole OS fits together, start here and follow the includes.
//
// Initialization order matters:
//   1. Serial + GDT + IDT       -- so we can log and handle exceptions ASAP.
//   2. Multiboot parsing        -- we need the memory map and framebuffer
//                                   info before we can set up memory or video.
//   3. Physical memory manager  -- needed before the heap can allocate pages.
//   4. Heap                     -- needed before anything calls `new`.
//   5. Framebuffer              -- needed before any drawing happens.
//   6. PIT, keyboard, mouse     -- input/timing drivers.
//   7. Desktop / compositor     -- the GUI, built on everything above.
// =============================================================================
#include "types.h"
#include "gdt.h"
#include "idt.h"
#include "multiboot.h"
#include "memory/pmm.h"
#include "memory/heap.h"
#include "drivers/serial.h"
#include "drivers/framebuffer.h"
#include "drivers/pit.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "gui/compositor.h"
#include "gui/desktop.h"
#include "libc/stdio.h"

namespace {

void print_banner() {
    kprintf("\n");
    kprintf("========================================\n");
    kprintf("   MyOS -- booting custom C++ kernel\n");
    kprintf("========================================\n");
}

// Drawn directly (bypassing the compositor) so it shows up even if later
// initialization steps fail -- a minimal "it's alive" splash screen.
void draw_splash_screen() {
    fb::fill_rect(0, 0, fb::width(), fb::height(), 0x1E2129);
    const char* title = "MyOS";
    int tx = (fb::width() - fb::text_width(title)) / 2;
    fb::draw_string(tx, fb::height() / 2 - 20, title, 0xEAECEF);
    const char* sub = "STARTING...";
    int sx = (fb::width() - fb::text_width(sub)) / 2;
    fb::draw_string(sx, fb::height() / 2, sub, 0x9AA0AC);
    fb::present();
}

} // namespace

extern "C" void kernel_main(uint32_t magic, uint32_t mb_info_addr) {
    serial::init();
    print_banner();

    if (magic != 0x36d76289) {
        kprintf("[FATAL] invalid multiboot2 magic: %x\n", magic);
        for (;;) { asm volatile ("cli; hlt"); }
    }

    gdt::init();
    idt::init();

    multiboot::parse(mb_info_addr);

    pmm::init();
    heap::init();

    if (!fb::init()) {
        kprintf("[FATAL] could not initialize graphics, halting\n");
        for (;;) { asm volatile ("cli; hlt"); }
    }
    draw_splash_screen();

    pit::init(100);          // 100 Hz system tick
    keyboard::init();
    mouse::init(fb::width(), fb::height());

    // Interrupts are now safe to enable: IDT is loaded and every driver that
    // registers a handler has done so above.
    asm volatile ("sti");

    pit::sleep_ms(400);      // let the splash screen linger briefly

    desktop::init();

    kprintf("[kernel] entering main loop\n");
    for (;;) {
        compositor::frame();
        // Yield to the next timer interrupt instead of busy-spinning --
        // keeps CPU usage sane and roughly caps the frame rate to the PIT
        // frequency until a proper vsync/frame-timing system exists.
        asm volatile ("hlt");
    }
}
