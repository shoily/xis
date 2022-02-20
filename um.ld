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
ENTRY(_um_start)

PHDRS {
      text PT_LOAD FILEHDR PHDRS;
      data PT_LOAD;
      bss PT_LOAD;
}

SECTIONS
{
    . = 0x10004000;
    .text :
    {*(.text); } : text
    .data :
    {*(.data); } : data
    .bss :
    {*(.bss); } : bss
}
