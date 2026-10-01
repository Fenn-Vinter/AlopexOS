/* ========================================================================== */
/* AlopexOS Linker Script                                                     */
/* ========================================================================== */

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    /* Higher-half base address mandated by Limine specification */
    . = 0xFFFFFFFF80000000;

    /* ====================================================================== */
    /* Limine Requests Section (Dedicated Loadable PHDR)                      */
    /* ====================================================================== */

    .limine_requests : AT(ADDR(.limine_requests) - 0xFFFFFFFF80000000)
    {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests.*))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(0x1000);

    /* ====================================================================== */
    /* Read-Only Section (Mapped to 'text' PHDR)                              */
    /* ====================================================================== */

    .text : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        *(.text .text.*)
    } :text

    . = ALIGN(0x1000);
    .rodata : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata .rodata.*)
    } :text

    /* ====================================================================== */
    /* Read-Write Section (Mapped to 'data' PHDR)                             */
    /* ====================================================================== */

    . = ALIGN(0x1000);
    .data : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data .data.*)
    } :data

    . = ALIGN(0x1000);
    .bss : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        *(COMMON)
        *(.bss .bss.*)
    } :data

    /* ====================================================================== */
    /* Discard Unwanted Metadata                                              */
    /* ====================================================================== */

    /DISCARD/ :
    {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}