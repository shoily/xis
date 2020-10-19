/*****************************************************************************/
/*  File: lock.h                                                             */
/*                                                                           */
/*  Description: Header file for synchonization primitives.                  */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Oct 16, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _LOCK_H
#define _LOCK_H

typedef struct _spin_lock {
    int val;
} spin_lock;

#define INIT_SPIN_LOCK(spinlock) memset(spinlock, sizeof(spin_lock), 0)

void spinlock_lock(spin_lock *lock);
void spinlock_unlock(spin_lock *lock);

#endif
