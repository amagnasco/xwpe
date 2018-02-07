#ifdef __linux__
/** \file WeLinux.c */
/*-------------------------------------------------------------------------*\
  <WeLinux.c> -- Linux specific routines for Xwpe

  Date      Programmer  Description
  04/11/98  Dennis      Created based on functions from "we_linux.c".
\*-------------------------------------------------------------------------*/

/* we_linux.c -- Created by Sebastiano Suraci */

#include "config.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include "WeLinux.h"
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  bioskey - Get the status of shift, alt, and control keys.

    Returns: A bit field of the following info
      Bit  Information
       3   Alt key
       2   Control key
       1   Left shift
       0   Right shift
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int
WpeLinuxBioskey (void)
{
    char c = 6;
    int status = 0;

    if (ioctl (STDIN_FILENO, TIOCLINUX, &c) == 0) {
        if (c & 0x01) {
            /* Right or left shift is pressed */
            status |= 0x03;
        }
        if (c & 0x04) {
            /* Control key is pressed */
            status |= 0x04;
        }
        if (c & 0x0A) {
            /* Alt key is pressed */
            status |= 0x08;
        }
    }
    return (status);
}

#endif
