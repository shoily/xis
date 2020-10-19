/*****************************************************************************/
/*  File: lock.c                                                             */
/*                                                                           */
/*  Description: Source file for synchonization primitives.                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Oct 16, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "lock.h"

void spinlock_lock(spin_lock *lock) {
    
    __asm__ __volatile__("movl $1, %%eax;"
                         "1:;"
                         "lock xchgl %0, %%eax;"
                         "testl %%eax, %%eax;"
                         "jne 1b;"
                         : "=m" (lock->val)
                         : "m" (lock->val)
                         : "%eax", "%ebx", "cc", "memory" );
}

void spinlock_unlock(spin_lock *lock) {

    __asm__ __volatile__("movl $0, %0;"
                         : "=m" (lock->val)
                         :
                         : "cc", "memory" );
}
