OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

/* Define Program Headers to split segments */
PHDRS
{
    text PT_LOAD FLAGS(5); /* RX: Read (4) + Execute (1) = 5 */
    data PT_LOAD FLAGS(6); /* RW: Read (4) + Write (2) = 6 */
}

SECTIONS
{
    . = 0xffffffff80000000;

    /* Assign .text to the 'text' segment (RX) */
    .text ALIGN(0x1000) :
    {
        *(.text*)
    } :text

    .limine_requests ALIGN(8) :
    {
        KEEP(*(.limine_requests))
    } :text /* Limine requests are typically read-only data, kept with text or rodata */

    /* Assign .rodata to the 'text' segment (RX) since it's read-only */
    .rodata ALIGN(0x1000) :
    {
        *(.rodata*)
    } :text

    /* Assign .data to the 'data' segment (RW) */
    .data ALIGN(0x1000) :
    {
        *(.data*)
    } :data

    /* Assign .bss to the 'data' segment (RW) */
    .bss ALIGN(0x1000) :
    {
        *(COMMON)
        *(.bss*)
    } :data
}   