OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0x80100000;
    .text : AT(ADDR(.text) - 0x80000000) {*(.text); }
    .data : AT(ADDR(.data) - 0x80000000) {*(.data); }
    .bss :  AT(ADDR(.bss) - 0x80000000) {*(.bss); }
    . = ALIGN(4096);
    .um : AT(ADDR(.um) - 0x80000000) {*(.um);}
    um_size = SIZEOF(um);
    . = ALIGN(4096);
    .mp_init : AT(ADDR(.mp_init) - 0x80000000) {*(.mp_init);}
    mp_init_size = SIZEOF(mp_init);
    .kernel_data ALIGN(4096) : AT(ADDR(.kernel_data) - 0x80000000) {*(.kernel_data); }
    . = ALIGN(4096);
    _kernel_pg_dir = .;
    _kernel_pg_table_0 = _kernel_pg_dir + 0x1000;
    _kernel_stack_0 = _kernel_pg_table_0 + 0x1000;
    _kernel_stack_0_start = _kernel_stack_0 + 0x2000;
    _kernel_private_data = _kernel_stack_0_start + 8;
    _kernel_heap_start = (_kernel_private_data + 0x1000) & ~(0xfff);
}
