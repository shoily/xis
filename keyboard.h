/*****************************************************************************/
/*  File: keyboard.c                                                         */
/*                                                                           */
/*  Description: Header file for handling keyboard interrupt                 */
/*               Converts set 2 scan code to ascii code.                     */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Nov 4, 2022                                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "type.h"

#define SPECIAL_KEY_CODE_START 0x80
enum special_key_code {
                       KEY_F1 = SPECIAL_KEY_CODE_START,
                       KEY_F2,
                       KEY_F3,
                       KEY_F4,
                       KEY_F5,
                       KEY_F6,
                       KEY_F7,
                       KEY_F8,
                       KEY_F9,
                       KEY_F10,
                       KEY_F11,
                       KEY_F12,
                       KEY_F13,
                       KEY_CAPS,
                       KEY_LSHIFT,
                       KEY_RSHIFT,
                       KEY_LCTRL,
                       KEY_RCTRL,
                       KEY_LALT,
                       KEY_RALT,
                       KEY_WIN,
                       KEY_BKSP,
                       KEY_PRSC,
                       KEY_SCROLL,
                       KEY_PAUSE,
                       KEY_INSERT,
                       KEY_HOME,
                       KEY_DELETE,
                       KEY_END,
                       KEY_PGUP,
                       KEY_PGDOWN,
                       KEY_UP,
                       KEY_DOWN,
                       KEY_LEFT,
                       KEY_RIGHT,
                       KEY_NUM
};

enum scan_special_keys {
                        SC_KEY_F9 = 0x1, // 01
                        SC_KEY_F5 = 0x3, // 03
                        SC_KEY_F3 = 0x4, // 04
                        SC_KEY_F1 = 0x5, // 05
                        SC_KEY_F2 = 0x6, // 06
                        SC_KEY_F12 = 0x7, // 07
                        SC_KEY_F10 = 0x9,// 09
                        SC_KEY_F8 = 0xa, // 0a
                        SC_KEY_F6 = 0xb, // 0b
                        SC_KEY_F4 = 0xc, // 0c
                        SC_KEY_LALT = 0x11, // 11
                        SC_KEY_LSHIFT = 0x12, // 12
                        SC_KEY_LCTRL = 0x14, // 14
                        SC_KEY_RCTRL = 0x14, // e0 14
                        SC_KEY_LWIN = 0x1f, // e0 1f
                        SC_KEY_CAPS = 0x58, // 58
                        SC_KEY_RSHIFT = 0x59, // 59
                        SC_KEY_BKSP = 0x66, // 66
                        SC_KEY_NUM = 0x77, // 77
                        SC_KEY_F11 = 0x78, // 78
                        SC_KEY_SCROLL = 0x7e, // 7e
                        SC_KEY_F7 = 0x83, // 83
};

void keyboard_init();
void keyboard_enable_interrupt();
void keyboard_handle_interrupt();
#endif
