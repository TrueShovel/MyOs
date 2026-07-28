/* =============================================================================
 * linker.ld -- Kernel memory layout
 * =============================================================================
 * Tells the linker where to place the kernel in memory. We load the kernel
 * at the 1 MiB mark (0x100000), which is the traditional, safe location for
 * a Multiboot2 kernel -- everything below 1 MiB is reserved for legacy BIOS/
 * real-mode structures.
 * ============================================================================= */

ENTRY(_start)

SECTIONS
{
    . = 1M;

    /* Multiboot header must appear early in the image so GRUB can find it. */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot2)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Arrays of C++ global constructor pointers, used by init_constructors() */
    .init_array BLOCK(4K) : ALIGN(4K)
    {
        __init_array_start = .;
        KEEP(*(.init_array))
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        __init_array_end = .;
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* End-of-kernel marker, used by the physical memory manager to know
     * where free memory begins. */
    . = ALIGN(4K);
    kernel_end_marker = .;
}
