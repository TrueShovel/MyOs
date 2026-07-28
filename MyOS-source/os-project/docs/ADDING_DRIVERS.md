# Adding a new driver

Drivers live in `drivers/` as a `.h`/`.cpp` pair exposing a small
namespace. They don't know about each other or about the GUI — they just
expose data (e.g. "here's the next queued keycode") or actions (e.g.
"draw this pixel"). This keeps every driver independently testable and
replaceable.

## Steps

1. **Create `drivers/yourdevice.h`** declaring an `init()` function and
   whatever query/action functions the rest of the kernel needs, inside a
   namespace matching the file name. Copy the shape of `drivers/pit.h` —
   it's the shortest complete example.

2. **Create `drivers/yourdevice.cpp`** with the implementation. If your
   device is interrupt-driven (most real hardware is), register a handler
   in `init()`:
   ```cpp
   #include "kernel/idt.h"

   namespace {
       void my_irq_handler(Registers* regs) {
           // read from the device's I/O ports, update internal state
       }
   }

   namespace yourdevice {
       void init() {
           idt::register_handler(32 + YOUR_IRQ_NUMBER, my_irq_handler);
       }
   }
   ```
   IRQ numbers 0-15 map to interrupt vectors 32-47 (see the table at the
   top of `kernel/idt.h`). Common ones: IRQ0 = PIT timer (32), IRQ1 =
   keyboard (33), IRQ12 = PS/2 mouse (44).

3. **Talk to hardware via `kernel/io.h`** (`inb`/`outb`/`inw`/`outw`) for
   any I/O-port-based device. If your device instead uses memory-mapped
   I/O (common for PCI devices), you'll read/write directly through a
   pointer to its base address instead — see `drivers/framebuffer.cpp`
   for that pattern (it writes through a pointer to the linear
   framebuffer rather than through ports).

4. **Call `yourdevice::init()` from `kernel/kernel_main.cpp`**, in the
   correct dependency order (after `idt::init()`, since you're
   registering an interrupt handler; after `pmm`/`heap::init()` if you
   allocate memory).

5. **Use `kprintf(...)` liberally** (from `libc/stdio.h`) while bringing a
   new driver up — it logs to the serial port, which is visible in your
   terminal via `make run`'s `-serial stdio`, and survives even if the
   graphical console later crashes.

## Example candidates for a first driver to add

- **RTC (real-time clock)** — replaces the taskbar's uptime counter with
  an actual wall-clock time. Small, self-contained, good first exercise:
  read `drivers/pit.cpp` first for the "PIC/port-based timer" pattern.
- **PCI enumeration** — a prerequisite for most further hardware support
  (network cards, sound cards, SATA). Walks PCI config space via ports
  0xCF8/0xCFC to discover attached devices.
- **AC97/HDA audio, e1000 network card, AHCI/SATA disk** — each is a
  natural next driver once PCI enumeration exists; see
  `docs/ROADMAP.md` for how these fit into the bigger picture.
