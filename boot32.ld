OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0X80100000;
    .text : AT(ADDR(.text) - 0x80000000) {*(.text); }
    .data : AT(ADDR(.data) - 0x80000000) {*(.data); }
    .bss :  AT(ADDR(.bss) - 0x80000000) {*(.bss); }
}
