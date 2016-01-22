#ifdef __linux__
/*-------------------------------------------------------------------------*\
  <WeLinux.c> -- Linux specific routines for Xwpe

  Date      Programmer  Description
  04/11/98  Dennis      Created based on functions from "we_linux.c".
\*-------------------------------------------------------------------------*/

/* we_linux.c -- Created by Sebastiano Suraci */

#include <sys/ioctl.h>

int WpeLinuxBioskey(void)
{
 char c;
 int status;

 c = 6;
 status = 0;
 if (ioctl(0, TIOCLINUX, &c) == 0)
 {
  if (c & 0x01)
  {
   /* Right or left shift is pressed */
   status |= 0x03;
  }
  if (c & 0x04)
  {
   /* Control key is pressed */
   status |= 0x04;
  }
  if (c & 0x0A)
  {
   /* Alt key is pressed */
   status |= 0x08;
  }
 }
 return(status);
}

#endif

