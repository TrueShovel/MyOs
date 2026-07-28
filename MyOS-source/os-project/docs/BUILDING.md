# Building MyOS

## Prerequisites

Tested on Ubuntu 24.04. Install the toolchain:

```bash
sudo apt-get update
sudo apt-get install -y \
    nasm \             # assembler for boot/boot.s and kernel/isr_stubs.s
    gcc-multilib \     # lets the host's gcc/g++ target 32-bit (-m32)
    g++-multilib \
    grub-pc-bin \      # grub-mkrescue and the BIOS boot images it embeds
    grub-common \
    xorriso \          # builds the ISO9660 filesystem grub-mkrescue needs
    mtools \           # used internally by grub-mkrescue
    qemu-system-x86    # to run the OS without real hardware
```

We deliberately build with the **host's regular gcc/g++ in `-m32`
freestanding mode** rather than a dedicated `i686-elf-*` cross-compiler.
A cross-compiler is the more "correct" long-term setup (see
docs/ROADMAP.md), but requires building binutils+gcc from source, which
takes much longer and isn't necessary for a 32-bit kernel: with
`-ffreestanding -nostdlib -fno-pic` and friends, the host compiler
produces a correct freestanding binary. If you outgrow this (e.g. you
want to target 64-bit or a different host architecture), building a real
cross-compiler is the standard next step — see the OSDev.org "GCC Cross
Compiler" article.

## Build commands

```bash
make            # builds build/kernel.bin only
make iso        # builds build/MyOS.iso (bootable CD image)
make run        # builds the ISO and boots it in QEMU with a display
make run-nogui  # same, but headless (no window) -- for servers/CI
make clean      # removes all build output
```

## What each build step actually does

1. **Assemble** `boot/boot.s` and `kernel/isr_stubs.s` with `nasm -f elf32`
   into 32-bit ELF object files.
2. **Compile** every `.cpp` file with:
   ```
   g++ -m32 -std=c++17 -ffreestanding -fno-exceptions -fno-rtti \
       -fno-stack-protector -mgeneral-regs-only -fno-pic -I. -c <file>
   ```
   - `-ffreestanding` — tells the compiler not to assume a hosted
     environment (no libc, no `main()` return value semantics, etc).
   - `-fno-exceptions -fno-rtti` — C++ exception unwinding and RTTI both
     need runtime support we don't have; disabling them also keeps
     generated code smaller and simpler.
   - `-mgeneral-regs-only` — forbids the compiler from emitting any
     FPU/SSE/MMX instructions. We never save/restore that CPU state on
     interrupts, so using it would corrupt data on a context switch; this
     flag makes it a compile error instead of a silent runtime bug.
   - `-fno-pic` — position-independent code needs a GOT/runtime relocator
     we don't have; the kernel is linked at a fixed address anyway.
3. **Link** all object files with `g++ -m32 -no-pie -T boot/linker.ld
   -nostdlib -static-libgcc ... -lgcc`:
   - `-T boot/linker.ld` — places the kernel per our layout, not the
     host's default one.
   - `-nostdlib` — don't link the host's C library or startup files.
   - `-lgcc` — **still needed** even though we exclude libc: it provides
     pure-arithmetic helper routines the compiler generates calls to,
     like 64-bit division on a 32-bit target (`__udivdi3`). This is
     compiler-support code, not an operating-system dependency, so it's
     fine in a freestanding kernel.
   - `-no-pie` — the host's gcc defaults to producing a position
     independent executable, which is incompatible with a kernel linked
     at a fixed physical address; this disables that default.
4. **Package**: `make iso` copies `kernel.bin` and `grub.cfg` into an
   ISO9660 tree and runs `grub-mkrescue`, which embeds a BIOS+El Torito
   boot image so the result boots on both legacy BIOS and (via GRUB's
   UEFI shim, if installed) UEFI firmware.

## Running

```bash
make run
```

This boots `build/MyOS.iso` in QEMU with a display window and forwards
the emulated serial port to your terminal (`-serial stdio`), so kernel
debug logs (`kprintf` output) appear right there alongside the graphical
window.

## Booting on real hardware

`build/MyOS.iso` is a standard bootable CD/USB image:

```bash
# ⚠️ this ERASES the target device -- double-check /dev/sdX!
sudo dd if=build/MyOS.iso of=/dev/sdX bs=4M status=progress && sync
```

Boot the machine from that USB drive (may require disabling Secure Boot
and/or enabling legacy/CSM boot in firmware settings, since this kernel
doesn't yet have a signed UEFI boot path). Keyboard/mouse should work via
any machine's PS/2 emulation; a real linear framebuffer will be set up by
GRUB the same way it is in QEMU. If the machine only exposes USB
keyboard/mouse with no PS/2 emulation, they won't respond yet — see
`docs/ROADMAP.md` (USB support).

## Troubleshooting

- **`nasm: command not found` / `grub-mkrescue: command not found`** —
  re-run the `apt-get install` line above.
- **Link error about missing `__udivdi3` or similar `__*di3` symbols** —
  make sure `-lgcc` appears *after* the object files on the link command
  (order matters to the linker); the provided Makefile already does this.
- **Kernel builds but QEMU shows a black screen** — check the serial log
  first (`make run` prints it to your terminal); if you see `[FATAL]
  could not initialize graphics`, your QEMU version may not honor the
  requested framebuffer tag — try adding `-vga std` to the QEMU
  invocation in the Makefile's `run` target.
- **Boots but keyboard/mouse don't respond in real hardware** — see "USB
  support" above; PS/2-only for now.
