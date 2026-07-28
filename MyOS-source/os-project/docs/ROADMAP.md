# Roadmap

This OS is an early-stage foundation, built so each of the following can
be added incrementally without rewriting what already exists. Roughly
ordered by how directly one enables the next.

## 1. Paging / virtual memory
**Why it's next:** almost everything else (real process isolation,
loading user programs, memory-mapped files) depends on it.
**Where it goes:** new `kernel/memory/paging.cpp`. The PMM (`pmm.cpp`)
already hands out physical frames by address, so paging is "just" building
page tables that map those frames into a virtual address space and
loading `CR3`/setting the paging bit in `CR0`. Once enabled, `heap.cpp`'s
static arena can be replaced with a heap that grows by mapping newly
allocated PMM frames on demand.

## 2. Preemptive multitasking / real process management
**Where it goes:** `kernel/process.cpp` (new) for a `Process`/`Thread`
struct and a simple round-robin scheduler; hook the scheduler tick into
`drivers/pit.cpp`'s existing IRQ0 handler. Needs paging first (step 1) for
process address-space isolation. Once this exists, apps could run as
separate processes instead of all executing inside the kernel's single
address space.

## 3. System calls
**Where it goes:** a new interrupt vector (e.g. 0x80, following the
classic x86 convention) registered via `idt::register_handler`, dispatched
to functions like `sys_open`, `sys_write` based on a register holding the
call number. Natural to add alongside process management, since syscalls
are how user-mode processes would ask the kernel to do things on their
behalf.

## 4. Real filesystem
**Where it goes:** `kernel/fs/` (new directory) with a small VFS
(virtual filesystem) interface, then a concrete implementation — FAT32 is
the traditional first choice (simple format, real-world compatible).
`apps/filemanager.cpp` is already structured so its demo listing can be
swapped for real VFS calls without changing its drawing/click code.
Needs a disk driver first (AHCI/SATA, or ATA/IDE for something simpler).

## 5. Networking
**Where it goes:** `drivers/` for a NIC driver (e.g. Intel e1000, widely
supported by QEMU/VirtualBox — a good first target), plus a new
`kernel/net/` for a minimal TCP/IP stack. This is a substantial project on
its own; many teaching OSes stop before this point.

## 6. Audio
**Where it goes:** `drivers/` for an AC97 or Intel HDA driver (both
common in VM emulated hardware), exposing a simple `write_samples()` API
that apps can push PCM audio into.

## 7. USB support
**Where it goes:** `drivers/usb/` for a host controller driver (UHCI is
the simplest to implement, though oldest; XHCI is what real modern
hardware has). This is what would make USB keyboards/mice work on
hardware without PS/2 emulation, and is a prerequisite for USB storage.

## 8. Security features
Once processes exist (step 2) and syscalls exist (step 3): user/kernel
privilege separation (ring 3 execution, currently everything runs in
ring 0), per-process memory permissions (via paging's page table flags),
and basic permission checks in the VFS (step 4).

## 9. Package management
Depends on having a filesystem (step 4) and a way to execute loaded
programs (step 2/3). At that point, a package manager is mostly userspace
tooling: a simple archive format + a manifest + a program that copies
files into place — genuinely one of the more approachable items on this
list once its prerequisites exist.

## 10. Additional filesystems
Once the VFS exists (step 4), additional filesystems (ext2, a simple
custom one, etc.) are each a self-contained module implementing the VFS
interface — no changes needed elsewhere.

## Smaller, always-welcome improvements
- Real-time clock (RTC) driver — replace the taskbar's uptime counter
  with actual wall-clock time (see `docs/ADDING_DRIVERS.md`).
- Window resizing (currently windows are drag-only, fixed size).
- A proper event queue instead of the compositor's current "poll every
  frame" input model — matters more once processes/syscalls exist and
  input needs to route across process boundaries.
- Lowercase and extended characters in `gui/font8x8.h`.
- Cross-compiler toolchain (`i686-elf-gcc`) instead of host `-m32` — more
  correct isolation from host headers/ABI quirks; see `docs/BUILDING.md`
  for why this wasn't required for the current 32-bit-only scope.
