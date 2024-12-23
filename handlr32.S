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

    .globl trap_handler_0
    .globl trap_handler_1
    .globl trap_handler_2
    .globl trap_handler_3
    .globl trap_handler_4
    .globl trap_handler_5
    .globl trap_handler_6
    .globl trap_handler_7
    .globl trap_handler_8
    .globl trap_handler_9
    .globl trap_handler_10
    .globl trap_handler_11
    .globl trap_handler_12
    .globl trap_handler_13
    .globl trap_handler_14
    .globl trap_handler_16
    .globl trap_handler_17
    .globl trap_handler_18
    .globl trap_handler_19
    .globl trap_handler_30
    
    .globl sys_call_handler_128

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

    .macro PROLOGUE trap_nr

    pusha

    movl %esp, %eax
    movl $0, -4(%eax)
    movl $0, -8(%eax)
    movl $0, -12(%eax)
    movl $0, -16(%eax)

    push %ds
    push %es
    push %fs
    push %gs

	pushl $\trap_nr

    movl %esp, %ebp
    // reg frame
    push %ebp

    // cleanup segment registers
    movl $0x40, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs

    // CUR_CPU on gs register
    xorl %eax, %eax
    movl %eax, %gs
    cmpl %eax, lapic_present
    je 1f
    pushl $0x20
    call lapic_read_register
    shr $24, %eax

    // pops off input parameter
    addl $4, %esp
    movl %eax, %gs
1:
    // reg_frame is already pushed into stack
    // should call handler

    .endm

    .macro EPILOGUE

    // adjusted for reg frame and trap_nr
    addl $8, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds

    popa

    // adjusted for code_nr
    addl $4, %esp

    iret

    .endm

    // macro for defining IRQ handler using PIC
    .macro DEFINE_IRQ_HANDLER irqn
    
    irq_handler_\irqn:
    // code_nr
    pushl $\irqn

    PROLOGUE 0
    call common_interrupt_handler
    EPILOGUE

    .endm


    // macro for defining system call handler
    .macro DEFINE_SYS_CALL_HANDLER syscallnr

    sys_call_handler_\syscallnr:
    pushl $\syscallnr

    PROLOGUE 0
    call common_sys_call_handler
    EPILOGUE

    .endm

    // macro for defining trap handler
    .macro DEFINE_TRAP_HANDLER trapnr
    trap_handler_\trapnr:

    PROLOGUE \trapnr
    call trap_handler
    EPILOGUE

    .endm

    // macro for defining trap handler
    .macro DEFINE_TRAP_NO_CODE_HANDLER trapnr
    trap_handler_\trapnr:
    // code_nr
    pushl $0

    PROLOGUE 0
    call trap_handler
    EPILOGUE

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
    DEFINE_SYS_CALL_HANDLER 128

    // define trap handler
    DEFINE_TRAP_NO_CODE_HANDLER 0
    DEFINE_TRAP_NO_CODE_HANDLER 1
    DEFINE_TRAP_NO_CODE_HANDLER 2
    DEFINE_TRAP_NO_CODE_HANDLER 3
    DEFINE_TRAP_NO_CODE_HANDLER 4
    DEFINE_TRAP_NO_CODE_HANDLER 5
    DEFINE_TRAP_NO_CODE_HANDLER 6
    DEFINE_TRAP_NO_CODE_HANDLER 7
    DEFINE_TRAP_HANDLER 8
    DEFINE_TRAP_NO_CODE_HANDLER 9
    DEFINE_TRAP_HANDLER 10
    DEFINE_TRAP_HANDLER 11
    DEFINE_TRAP_HANDLER 12
    DEFINE_TRAP_HANDLER 13
    DEFINE_TRAP_HANDLER 14
    DEFINE_TRAP_NO_CODE_HANDLER 16
    DEFINE_TRAP_NO_CODE_HANDLER 17
    DEFINE_TRAP_HANDLER 18
    DEFINE_TRAP_NO_CODE_HANDLER 19
    DEFINE_TRAP_HANDLER 30
