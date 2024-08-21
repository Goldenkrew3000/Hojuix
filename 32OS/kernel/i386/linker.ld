ENTRY(_start)

SECTIONS
{
    . = 2M;
    startkernel = .; /* Start of the kernel */

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        sbss = .; /* BSS Start */
        *(COMMON)
        *(.bss)
        ebss = .; /* BSS End */
    }

    endkernel = .; /* End of the kernel */
}
