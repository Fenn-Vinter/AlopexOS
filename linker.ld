/* Target architecture and entry symbol */
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
    text    PT_LOAD;
    rodata  PT_LOAD;
    data    PT_LOAD;
    bss     PT_LOAD;
}

SECTIONS
{
    /* Higher-half base address with higher-half offset */
    . = 0xFFFFFFFF80000000;

    .text : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        *(.text .text.*)
    } :text

    . = ALIGN(0x1000);
    .rodata : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata .rodata.*)
    } :rodata

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
    } :bss

    /DISCARD/ :
    {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame)
    }
}