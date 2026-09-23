.section .text
.global _start

_start:
    cli
    lea stack_top(%rip), %rsp
    andq $-16, %rsp
    call kmain

.hang:
    hlt
    jmp .hang

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top: