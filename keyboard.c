/*****************************************************************************/
/*  File: keyboard.c                                                         */
/*                                                                           */
/*  Description: Source file for handling keyboard interrupt                 */
/*               Converts set 2 scan code to ascii code.                     */
/*                                                                           */
/*  Author: Shoily O Rahman <shoily@gmail.com>                               */
/*                                                                           */
/*  Date: Nov 4, 2022                                                        */
/*                                                                           */
/*****************************************************************************/

#include "keyboard.h"
#include "util.h"
#include "lock.h"

struct keyboard_scancode_state {
    bool lshift_down:1;
    bool rshift_down:1;
    bool lctrl_down:1;
    bool rctrl_down:1;
    bool lalt_down:1;
    bool ralt_down:1;
    bool caps:1;
    bool break_code:1;
    bool break_nested_code:1;
    bool e0_code:1;
    bool e0_nested_code:1;
    bool e1_code:1;
    bool e1_nested_code:1;
    bool lwin_down:1;
    bool rwin_down:1;
    u8 prev_code;
};

#define KEYBOARD_BUFFER_SIZE 10
char kbd_buffer[KEYBOARD_BUFFER_SIZE];
struct ring_buffer ring_buffer_kbd;

struct keyboard_scancode_state kbd_state;

bool keyboard_controller_8042_present = false;
spinlock spinlock_kbd;

u8 conversion_scan_ascii[] = {
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              9, // 0d (TAB)
                              '`', // 0e
                              0, // 0f
                              0, // 10
                              0, // 11
                              0, // 12
                              0, // 13
                              0, // 14
                              'q', // 15
                              '1', // 16
                              0, // 17
                              0, // 18
                              0, // 19
                              'z', // 1a
                              's', // 1b
                              'a', // 1c
                              'w', // 1d
                              '2', // 1e
                              0, // 1f
                              0, // 20
                              'c', // 21
                              'x', // 22
                              'd', // 23
                              'e', // 24
                              '4', // 25
                              '3', // 26
                              0, // 27
                              0, // 28
                              ' ', // 29
                              'v', // 2a
                              'f', // 2b
                              't', // 2c
                              'r', // 2d
                              '5', // 2e
                              0, // 2f
                              0, // 30
                              'n', // 31
                              'b', // 32
                              'h', // 33
                              'g', // 34
                              'y', // 35
                              '6', // 36
                              0, // 37
                              0, // 38
                              0, // 39
                              'm', // 3a
                              'j', // 3b
                              'u', // 3c
                              '7', // 3d
                              '8', // 3e
                              0, // 3f
                              0, // 40
                              ',', // 41
                              'k', // 42
                              'i', // 43
                              'o', // 44
                              '0', // 45
                              '9', // 46
                              0, // 47
                              0, // 48
                              '.', // 49
                              '/', // 4a
                              'l', // 4b
                              ';', // 4c
                              'p', // 4d
                              '-', // 4e
                              0, // 4f
                              0, // 50
                              0, // 51
                              '\'', // 52
                              0, // 53
                              '[', // 54
                              '=', // 55
                              0, // 56
                              0, // 57
                              0, // 58
                              0, // 59
                              10, // 5a (ENTER)
                              ']', // 5b
                              0, // 5c
                              '\\', // 5d
                              0, // 5e
                              0, // 5f
                              0, // 60
                              0, // 61
                              0, // 62
                              0, // 63
                              0, // 64
                              0, // 65
                              0, // 66
                              0, // 67
                              0, // 68
                              0, // 69
                              0, // 6a
                              0, // 6b
                              0, // 6c
                              0, // 6d
                              0, // 6e
                              0, // 6f
                              0, // 70
                              0, // 71
                              0, // 72
                              0, // 73
                              0, // 74
                              0, // 75
                              27, // 76 (ESC)
};

#define SC_E0 0xe0
#define SC_E1 0xe1
#define SC_BREAK 0xf0

#define set_key_code(key) key_code = kbd_state.break_code ? 0 : key

u8 kbd_process_scan_code(u8 scan_code) {
    u8 key_code = 0;

    switch(scan_code) {
    case SC_E0:
        if (!kbd_state.e0_code) {
            kbd_state.e0_code = true;
        } else {
            kbd_state.e0_nested_code = 1;
        }
        break;
    case SC_E1:
        if (!kbd_state.e1_code) {
            kbd_state.e1_code = true;
        } else {
            kbd_state.e1_nested_code = 1;
        }
        break;
    case SC_BREAK:
        kbd_state.break_code = true;
        break;
    case SC_KEY_LSHIFT:
        kbd_state.lshift_down = !kbd_state.break_code;
        set_key_code(KEY_LSHIFT);
        kbd_state.break_code = false;
        break;
    case SC_KEY_RSHIFT:
        kbd_state.rshift_down = !kbd_state.break_code;
        set_key_code(KEY_RSHIFT);
        kbd_state.break_code = false;
        break;
    case SC_KEY_CAPS:
        kbd_state.caps = kbd_state.break_code ? kbd_state.caps : !kbd_state.caps;
        set_key_code(KEY_CAPS);
        kbd_state.break_code = false;
        break;
    default:
        key_code = conversion_scan_ascii[scan_code];
        if (key_code) {
            if (kbd_state.break_code) {
                key_code = 0;
                kbd_state.break_code = false;
                goto out;
            }
            if (key_code >= 'a' && key_code <= 'z') {
                bool caps = (kbd_state.lshift_down | kbd_state.rshift_down) ^ kbd_state.caps;
                key_code = caps ? key_code - 32 : key_code;
            }
        }
    };

 out:
    kbd_state.prev_code = scan_code;
    return key_code;
}

void keyboard_init() {
    INIT_SPIN_LOCK(&spinlock_kbd);

    ring_buffer_init(&ring_buffer_kbd, kbd_buffer, KEYBOARD_BUFFER_SIZE);

    memset(&kbd_state, 0, sizeof(kbd_state));

    // enable keyboard controller but disable the interrupt
    __asm__ __volatile__("1:"
                         "inb $0x64, %%al;"
                         "testb $0x2, %%al;"
                         "jz 1f;"
                         "inb $0x60, %%al;"
                         "jmp 1b;"
                         "1:"
                         "movb $0x60, %%al;"
                         "outb %%al, $0x64;"
                         "movb $0x60, %%al;" // turn on first PS/2 port interrupt and translation
                         "outb %%al, $0x60;"
                         "movb $0xae, %%al;"
                         "outb %%al, $0x64;"
                         :
                         :
                         : "%eax");
}

void keyboard_enable_interrupt() {
    __asm__ __volatile__("1:"
                         "inb $0x64, %%al;"
                         "testb $0x2, %%al;"
                         "jz 1f;"
                         "inb $0x60, %%al;"
                         "jmp 1b;"
                         "1:"
                         "movb $0x60, %%al;"
                         "outb %%al, $0x64;"
                         "movb $0x1, %%al;"
                         "outb %%al, $0x60;"
                         :
                         :
                         : "%eax");
}

#define KEYBOARD_DEBUG

void keyboard_handle_interrupt() {
    u8 scan_code = 0;
    u8 key_code;
    __asm__ __volatile__("inb $0x64, %%al;"
                         "testb $0x1, %%al;"
                         "jz 1f;"
                         "inb $0x60, %%al;"
                         "movb %%al, %0;"
                         "1:"
                         : "=r" (scan_code)
                         :
                         : "%eax");

    key_code = kbd_process_scan_code(scan_code);
    if (key_code >= SPECIAL_KEY_CODE_START)
        ring_buffer_put_elem(&ring_buffer_kbd, 0);
    ring_buffer_put_elem(&ring_buffer_kbd, key_code);

#ifdef KEYBOARD_DEBUG
    static int r = 0;
    static char s[7];
    if (key_code >= '0' && key_code <= 'z') {
        if (r == 6) {
            int i;
            for (i=1;i<6;i++)
                s[i-1] = s[i];
            s[5] = key_code;
        } else {
            s[r++] = key_code;
            s[r] = 0;
        }
        print_vga_fixed(s, 130, 0);
    }
#endif
}
