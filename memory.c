/*****************************************************************************/
/*  File: memory.c                                                           */
/*                                                                           */
/*  Description: Source file for memory allocation code.                     */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: July 9, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#include "memory.h"

extern int _kernel_heap_start;

char* heap_ptr;

void init_memory() {

    heap_ptr = (char*)_kernel_heap_start;
}

void *alloc_mem(int size, int alignment) {

    char *mem_start = (char*)heap_ptr;

    mem_start = (char*)(((int)heap_ptr + (alignment-1)) & ~(alignment-1));
    heap_ptr = mem_start + size;

    return mem_start;    
}
