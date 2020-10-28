OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0x80100000;
    .text : {*(.text); }
    .data : {*(.data); }
    .bss :  {*(.bss); }
    um_size = SIZEOF(um);
    . = ALIGN(8);
    .mp_init : {*(.mp_init); }
    . = ALIGN(4096);
    _kernel_pg_dir = .;
    _kernel_pg_dir_end = _kernel_pg_dir + (0x1000 * 32);
    _kernel_pg_table_0 = _kernel_pg_dir_end;
    _kernel_pg_table_0_end = _kernel_pg_table_0 + 0x1000;
    _kernel_stack_0 = _kernel_pg_table_0_end + 0x1000;
    _kernel_stack_0_start = _kernel_stack_0 + 0x2000;
    _kernel_private_data = _kernel_stack_0_start + (0x2000 * 32);
    _kernel_heap_start = (_kernel_private_data + 0x1000) & ~(0xfff);
    _end_kernel = .;
}
