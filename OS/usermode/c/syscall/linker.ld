ENTRY(main)  /* Entry point symbol */

/* Adjust based on your OS's user memory layout */

SECTIONS {
    .text 0x400000: {
        *(.text)      /* Code */
        *(.text.*)
    }

    .rodata 0x800000: {  /* Read-only data (4-byte aligned) */
        *(.rodata)
        *(.rodata.*)
    }

    .data 0x900000: {    /* Initialized data */
        *(.data)
        *(.data.*)
    }

    /*
    .bss : ALIGN(4) {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
	    */
}
