/*****************************************************************************/
/*  File: handlr32.S                                                         */
/*                                                                           */
/*  Description: x86 32 bit interrupt, trap and fault handler.               */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: June 15, 2020                                                      */
/*                                                                           */
/*****************************************************************************/

    .code32
    .text

    .globl common_trap_handler
    
    .globl system_call_handler_3

    .globl irq_handler_0
    .globl irq_handler_1
    .globl irq_handler_2
    .globl irq_handler_3
    .globl irq_handler_4
    .globl irq_handler_5
    .globl irq_handler_6
    .globl irq_handler_7
    .globl irq_handler_8
    .globl irq_handler_9
    .globl irq_handler_10
    .globl irq_handler_11
    .globl irq_handler_12
    .globl irq_handler_13
    .globl irq_handler_14
    .globl irq_handler_15


    // macro for defining IRQ handler
    .macro DEFINE_IRQ_HANDLER irqn
    
    irq_handler_\irqn:

    pusha

    movb $0x20,%al
    
    movb $\irqn, %ah
    cmpb $8, %ah
    jl 1f
    
    outb %al,$0xa0
1:
    
    outb %al,$0x20

    push $\irqn    
    call common_interrupt_handler
    addl $4, %esp

    popa

    iret
    
    .endm


    // macro for defining system call handler
    .macro DEFINE_SYS_CALL_HANDLER syscallnr

    sys_call_handler_\syscallnr:
    pushl $\syscallnr
    call common_sys_call_handler
    addl $4, %esp

    popa
    
    .endm


    // define IRQ handlers 0-15
    DEFINE_IRQ_HANDLER 0
    DEFINE_IRQ_HANDLER 1
    DEFINE_IRQ_HANDLER 2
    DEFINE_IRQ_HANDLER 3
    DEFINE_IRQ_HANDLER 4
    DEFINE_IRQ_HANDLER 5
    DEFINE_IRQ_HANDLER 6
    DEFINE_IRQ_HANDLER 7
    DEFINE_IRQ_HANDLER 8
    DEFINE_IRQ_HANDLER 9
    DEFINE_IRQ_HANDLER 10
    DEFINE_IRQ_HANDLER 11
    DEFINE_IRQ_HANDLER 12
    DEFINE_IRQ_HANDLER 13
    DEFINE_IRQ_HANDLER 14
    DEFINE_IRQ_HANDLER 15

    // define system call handlers
    DEFINE_SYS_CALL_HANDLER 3
    
    
common_trap_handler:
    pushl $0
    pushl $0

    call trap_handler

    addl $8,%esp
    iret
