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

typedef struct _spinlock {
    int val;
} spinlock;

#define INIT_SPIN_LOCK(lock) memset(lock, sizeof(spinlock), 0)

void spinlock_lock(spinlock *lock);
void spinlock_unlock(spinlock *lock);

#endif
