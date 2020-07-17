/*****************************************************************************/
/*  File: memory.h                                                           */
/*                                                                           */
/*  Description: Header file for memory allocation code.                     */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 9, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef MEMORY_H
#define MEMORY_H

void init_memory();
void *alloc_mem(int size, int alignment);

#endif
