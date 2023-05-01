OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0x80100000;
    _kernel_section_start = .;
    .text : {*(.text); }
    .data : {*(.data); }
    .bss :  {*(.bss); }
    . = ALIGN(8);
    .um_init : {*(.um_init); }
    . = ALIGN(8);
    .mp_init : {*(.mp_init); }
    . = ALIGN(0x1000);
    _kernel_stack_0 = .;
    _kernel_stack_0_start = _kernel_stack_0 + 0x2000;
    _kernel_stack_end = _kernel_stack_0_start + (0x2000 * 32);
    . = _kernel_stack_end;
    . = ALIGN(0x1000);
    _master_kernel_pg_dir = .;
    _kernel_pg_dir = _master_kernel_pg_dir + 0x1000;
    _kernel_pg_dir_end = _kernel_pg_dir + (0x1000 * 32);
    _kernel_pg_table_0 = _kernel_pg_dir_end;
    _kernel_pg_table_0_end = _kernel_pg_table_0 + 0x1000;
}
