/*****************************************************************************/
/*  File: mpinit.ld                                                          */
/*                                                                           */
/*  Description: ld script file for initializing multiple (application)      */
/*  processors.                                                              */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Oct 7, 2020                                                        */
/*                                                                           */
/*****************************************************************************/

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

/* This will overwrite default boot sector */
ENTRY(_start)

PHDRS {
      text PT_LOAD FILEHDR PHDRS;
      data PT_LOAD;
      bss PT_LOAD;
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
    . = 0x80007D00;
    .high : AT(0x7D00) {*(.high); }
}
