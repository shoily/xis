/*****************************************************************************/
/*  File: um.S                                                               */
/*                                                                           */
/*  Description: Usermode code.                                              */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Jun 20, 2021                                                        */
/*                                                                           */
/*****************************************************************************/

    .code32
    .text

.globl _um_start
_um_start:

    push %ebp
    mov %esp,%ebp
    sub  $0x4,%esp
    movl $0x1,-0x4(%ebp)
loop:
    int $0x80
    mov -0x4(%ebp),%eax
    inc %eax
    mov %eax,-0x4(%ebp)
    jmp loop
    nop
    leave
    ret  


