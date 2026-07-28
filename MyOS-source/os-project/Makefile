# =============================================================================
# Makefile -- MyOS build system
# =============================================================================
# Targets:
#   make            build build/kernel.bin
#   make iso        build build/MyOS.iso  (bootable CD image via GRUB)
#   make run        build the ISO and boot it in QEMU
#   make run-nogui  same, but without a graphical QEMU window (for CI/servers)
#   make clean      remove all build output
#
# See docs/BUILDING.md for a full walkthrough and troubleshooting tips.
# =============================================================================

CXX      := g++
AS       := nasm
LD       := ld

BUILD    := build
ISO_DIR  := iso

# -ffreestanding: no hosted standard library assumptions.
# -fno-exceptions/-fno-rtti: keeps the runtime simple (no unwind tables/typeid).
# -mgeneral-regs-only: forbids the compiler from emitting any FPU/SSE/MMX
#   instructions, since we never save/restore that CPU state on interrupts.
#   (We don't use floating point anywhere in the kernel, so this is free.)
CXXFLAGS := -m32 -std=c++17 -ffreestanding -fno-exceptions -fno-rtti \
            -fno-stack-protector -mgeneral-regs-only -fno-pic \
            -Wall -Wextra -O2 -I. -c

ASFLAGS  := -f elf32
# We link with g++ (not raw ld) so it automatically locates libgcc, which
# supplies compiler-generated helper routines like 64-bit division on a
# 32-bit target (__udivdi3 etc). -nostdlib still excludes libc/crt0/crtbegin,
# so the kernel remains fully freestanding aside from those pure-arithmetic
# helpers.
LDFLAGS  := -m32 -no-pie -fno-pic -T boot/linker.ld -nostdlib -static-libgcc

# --- Source discovery -------------------------------------------------------
CPP_SOURCES := $(shell find kernel drivers libc gui apps -name '*.cpp')
ASM_SOURCES := boot/boot.s kernel/isr_stubs.s

CPP_OBJECTS := $(patsubst %.cpp,$(BUILD)/%.o,$(CPP_SOURCES))
ASM_OBJECTS := $(patsubst %.s,$(BUILD)/%.o,$(ASM_SOURCES))
OBJECTS     := $(ASM_OBJECTS) $(CPP_OBJECTS)

KERNEL := $(BUILD)/kernel.bin
ISO    := $(BUILD)/MyOS.iso

.PHONY: all iso run run-nogui clean

all: $(KERNEL)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) -lgcc
	@echo "Kernel built: $@"

iso: $(KERNEL)
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	cp boot/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR) 2>&1 | grep -v "^xorriso" || true
	@echo "ISO built: $(ISO)"

run: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M -serial stdio

run-nogui: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M -serial stdio -display none

clean:
	rm -rf $(BUILD) $(ISO_DIR)/boot/kernel.bin $(ISO_DIR)/boot/grub/grub.cfg
