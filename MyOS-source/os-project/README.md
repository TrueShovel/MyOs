# MyOS

Scroll down to the very bottom if you are curious about your License using the source code.


A small, educational operating system written from scratch in C++. It boots
on real x86 hardware or in any BIOS-compatible virtual machine (QEMU,
VirtualBox, VMware), brings up a custom kernel, and launches a graphical
desktop environment with a window manager, taskbar, and a few built-in apps.

This is **not** Linux-based — it uses a custom kernel written entirely for
this project (see `docs/ARCHITECTURE.md` for why, and what "Linux kernel"
would have meant here). It's designed to be read, understood, and extended
by someone with intermediate C++ knowledge; every non-obvious line has a
comment explaining *why*, not just *what*.

## Quick start

```bash
# One-time setup (Ubuntu/Debian) -- installs the cross-toolchain-equivalent
# tools needed to build and run the OS:
sudo apt-get install -y nasm grub-pc-bin grub-common xorriso mtools \
                         gcc-multilib g++-multilib qemu-system-x86

# Build and boot in QEMU:
make run
```

You should see a splash screen, then a desktop with a taskbar (Terminal,
Files, Settings) and a terminal window already open. Click the taskbar
buttons to open more windows; drag windows by their title bar; type in the
terminal (try `help`).

See `docs/BUILDING.md` for the full build walkthrough, flag-by-flag
explanations, and how to boot on real hardware.

## What's actually implemented

This is a genuine, from-scratch OS — not a simulation — but it is an
**early-stage educational kernel**, not a production system. Being upfront
about scope:

| Area | Status |
|---|---|
| Boot (GRUB/Multiboot2), custom kernel | Working |
| GDT, IDT, CPU exceptions, hardware IRQs | Working |
| Physical memory manager (bitmap allocator) | Working |
| Kernel heap (`kmalloc`/`new`) | Working |
| Paging / virtual memory | **Not yet** — see roadmap |
| Preemptive multitasking / processes | **Not yet** — see roadmap |
| Graphics (linear framebuffer), GUI, window manager | Working |
| PS/2 keyboard + mouse | Working |
| Taskbar, draggable/closable windows, 3 demo apps | Working |
| Real filesystem | **Not yet** — File Manager shows a demo listing |
| Networking, audio, USB, package manager | **Not yet** — architected for, not built |

Every "not yet" above is a documented, scoped task in `docs/ROADMAP.md`
with pointers to where the code would go — the architecture was chosen
specifically so these can be added incrementally without rewrites.

## Project layout

```
boot/       Bootloader glue: Multiboot2 header, entry assembly, linker script
kernel/     Core kernel: GDT, IDT/interrupts, memory management, entry point
drivers/    Hardware drivers: framebuffer, keyboard, mouse, PIT timer, serial
libc/       Minimal freestanding C++ standard library replacement
gui/        Window manager, compositor, taskbar, desktop, font
apps/       Built-in applications (Terminal, File Manager, Settings)
docs/       Architecture notes, build guide, how to add drivers/apps, roadmap
```

Each directory has a single, clear responsibility (see
`docs/ARCHITECTURE.md`), so you can usually guess where new code belongs.

## License

You Are allowed to use the source code for your own OS development, however, if you use my branding for your own OS,
I will request you to take my branding off of your OS and will have you create your own branding for your OS.
