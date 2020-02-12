OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

PHDRS {
      text PT_LOAD FILEHDR PHDRS;
      data PT_LOAD;
      bss PT_LOAD;
      bootsig PT_LOAD;
}

SECTIONS
{
    . = 0X7C00;
    .text : 
    {*(.text); } : text
    .data : 
    {*(.data); } : data
    .bss : 
    {*(.bss); } : bss
    . = 0x7DFE;
    .bootsig :
    {*(.bootsig); } : bootsig
}
