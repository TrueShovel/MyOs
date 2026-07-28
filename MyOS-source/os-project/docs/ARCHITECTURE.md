# Architecture

This document explains what each part of the OS does, why it's built the
way it is, and where to look when you want to change something.

## Why a custom kernel instead of Linux?

The prompt asked for "a Linux kernel, if not available, make your own
custom kernel." Embedding the actual Linux kernel isn't something that can
be authored from scratch in a project like this — Linux is ~30 million
lines of code developed by tens of thousands of contributors over 30+
years, and "using" it as a component means either building a full
Buildroot/Yocto-style Linux distribution (a very different, much larger
project centered on configuring an existing kernel rather than writing an
OS) or writing a custom kernel that boots a Linux-compatible userspace
(a multi-year systems project on its own). Given the goal of a clean,
educational, from-scratch C++ codebase, this project takes the explicitly
offered fallback: **a custom kernel**, small enough to read in an
afternoon and extend piece by piece.

## Boot flow

```
BIOS/UEFI -> GRUB (reads boot/grub.cfg) -> boot/boot.s (_start)
          -> init_constructors() [kernel/crt.cpp]
          -> kernel_main() [kernel/kernel_main.cpp]
          -> subsystem init (gdt, idt, memory, drivers)
          -> desktop::init() [gui/desktop.cpp]
          -> infinite loop calling compositor::frame()
```

We use **GRUB + Multiboot2** rather than writing a raw boot sector. This is
a deliberate, standard choice (used by most hobby/teaching OS projects):
writing your own bootloader means re-implementing FAT/ISO9660 parsing,
disk drivers, and A20-line/protected-mode setup before you can even start
on the OS itself. GRUB already does all of that reliably, on both real
BIOS and UEFI hardware, and hands us a clean, documented info structure
(memory map, framebuffer address, etc). Nothing about the kernel *design*
depends on GRUB, though — `boot/boot.s` and `boot/linker.ld` are the only
files that would change if you later wrote a custom bootloader.

## Directory-by-directory

### `boot/`
- `boot.s` — the Multiboot2 header and `_start` entry point. Sets up a
  stack, runs C++ global constructors, then calls `kernel_main`.
- `linker.ld` — places the kernel at the 1 MiB physical address mark
  (standard for Multiboot2 kernels) and defines section layout.
- `grub.cfg` — the GRUB boot menu, copied into the ISO by `make iso`.

### `kernel/`
- `types.h`, `io.h` — foundational, dependency-free headers every other
  file builds on (fixed-width integers, port I/O).
- `gdt.cpp` — sets up flat-model segmentation (required by x86 protected
  mode; real memory protection here is meant to come from paging, see
  Roadmap).
- `idt.cpp` / `isr_stubs.s` — the interrupt system. `isr_stubs.s` is
  assembly because the CPU jumps to a raw address on interrupt with no
  C++ calling convention set up yet; it saves registers and calls into
  `isr_common_handler` in `idt.cpp`, which dispatches to whichever driver
  registered a handler for that interrupt number.
- `multiboot.cpp` — parses the tagged info structure GRUB hands us
  (memory map, framebuffer) once at boot.
- `memory/pmm.cpp` — physical memory manager: a bitmap tracking which
  4 KiB page frames are free. Simple and correct; the natural next step
  (documented in the roadmap) is a buddy allocator for less fragmentation.
- `memory/heap.cpp` — `kmalloc`/`kfree` (and `new`/`delete`) via a
  free-list allocator over a static arena. See the note in that file about
  why it's static rather than paged/growable yet.
- `kernel_main.cpp` — the entry point; brings up every subsystem in
  dependency order and runs the main loop. **Start reading here.**
- `crt.cpp` — the tiny bit of C++ ABI plumbing (constructor running, pure
  virtual stub, static-init guards) that a hosted program gets for free
  from its C runtime, which we don't have.

### `drivers/`
Each driver is a self-contained `.h`/`.cpp` pair exposing a small
namespace API (`init()`, plus whatever the driver provides). None of them
know about each other — `keyboard.cpp` doesn't know the GUI exists, it
just fills a character queue. This is what makes drivers easy to add or
swap (see `docs/ADDING_DRIVERS.md`).

- `serial.cpp` — COM1 debug logging (see it live via `-serial stdio`).
- `framebuffer.cpp` — draws into an off-screen buffer and presents it in
  one `memcpy` per frame, avoiding tearing/flicker.
- `pit.cpp` — the system timer; drives `sleep_ms` and the taskbar clock.
- `keyboard.cpp`, `mouse.cpp` — PS/2 drivers, chosen because they're
  interrupt-driven, simple to program, and universally emulated (real USB
  keyboards/mice also work through BIOS/UEFI's PS/2 emulation on most
  hardware). USB is a documented roadmap item.

### `libc/`
A tiny freestanding standard library replacement: `string.cpp` (memcpy,
strlen, etc.) and `stdio.cpp` (`kprintf`, formatted debug output). Add
functions here as the rest of the OS needs them — don't reach for
`<cstring>`/`<cstdio>`, they assume a hosted environment we don't have.

### `gui/`
- `window.cpp` — a single window: draws its frame/title bar and delegates
  its content area to an `App`.
- `compositor.cpp` — the window manager. Owns the window list and
  z-order, routes mouse/keyboard input, and draws one frame
  (background -> windows -> taskbar -> cursor -> present).
- `taskbar.cpp` — the launcher bar and clock.
- `desktop.cpp` — bootstraps the above and registers the built-in apps.
- `font8x8.h` — a small original bitmap font (see `docs/ADDING_APPS.md`
  if you need more characters than it currently covers).

### `apps/`
Each app implements the `App` interface from `gui/window.h`
(`draw`, `on_key`, `on_click`, `title`). `apps/terminal.cpp` is the best
one to read first — it's a complete, working example. See
`docs/ADDING_APPS.md` to add your own.

## Design decisions worth knowing about

- **32-bit protected mode, no paging (yet).** Keeping addressing flat
  (physical == linear) removes an entire subsystem's worth of complexity
  from the first version of the OS, at the cost of no per-process memory
  isolation. This is the single biggest documented roadmap item — see
  `docs/ROADMAP.md` — and the PMM was already written in terms of page
  frames so that adding a paging layer on top doesn't require rewriting
  it.
- **Cooperative, single "process" execution model.** There is no
  preemptive scheduler yet; the compositor's frame loop *is* the whole
  running program. Real process management (separate address spaces,
  context switching, a scheduler) is scoped in the roadmap as a
  self-contained addition to `kernel/` that the rest of the OS doesn't
  need to change to support.
- **Mode: linear framebuffer via Multiboot2, not raw VGA registers.**
  This works identically on real UEFI/BIOS hardware and in every VM,
  supports full 32-bit color at a real resolution, and needs no
  mode-switching code of our own.
