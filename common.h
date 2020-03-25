/*****************************************************************************/
/*  File: common.h                                                           */
/*                                                                           */
/*  Description: Header file for common/utility functions                    */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Mar 15, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

#define memset(address, size, c) {for(unsigned int i=0;i<size;i++) ((char*)address)[i]=c;}

int strlen(char *p);
void itoa(int val, char *str, int base);
void itoll(long long val, char *str, int base);

#endif // _COMMON_H
