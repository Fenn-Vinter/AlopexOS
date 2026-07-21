OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    . = 0xffffffff80000000;

    .text ALIGN(0x1000) :
    {
        *(.text*)
    }

    .limine_requests ALIGN(8) :
    {
        KEEP(*(.limine_requests))
    }

    .rodata ALIGN(0x1000) :
    {
        *(.rodata*)
    }

    .data ALIGN(0x1000) :
    {
        *(.data*)
    }

    .bss ALIGN(0x1000) :
    {
        *(COMMON)
        *(.bss*)
    }
}