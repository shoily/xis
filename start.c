/*****************************************************************************/
/*  File: start.c                                                            */
/*                                                                           */
/*  Description: Kernel initialization code.                                 */
/*  start_kernel routine is called from boot32.S.                            */
/*                                                                           */
/*  Author: Shoily O Rahman                                                  */
/*                                                                           */
/*  Date: Feb 11, 2020                                                       */
/*                                                                           */
/*****************************************************************************/

typedef int bool;
#define true 1
#define false 0

#define VIDEO_BUFFER (0x800b8000)
int vga_buffer_index = 0;

//
// Outputs a NULL terminated string in VGA buffer
//
void print_vga(char *c, bool newline) {

  unsigned short *p = (unsigned short *)(VIDEO_BUFFER+vga_buffer_index);
  
  while(*c) {
    
    *p = (4 << 8 | *c);
    c++;
    vga_buffer_index += 2;
    p++;
  }

  if (newline) {
    vga_buffer_index += (160 - vga_buffer_index);
  }
}

//
//  Start kernel routine
//

int start_kernel(void) {

  print_vga("Kernel started", true);
  
  return 0;
}
