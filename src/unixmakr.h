/** \file unixmakr.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"

#ifdef NOSTRSTR
char *strstr (char *s1, char *s2);
char *getcwd (char *dir, int n);
#endif


/**
 * tputs is necessary to add padding for termcap or terminfo
 * capabilities. For curses or ncurses this function is not
 * necessary
 */
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
#define e_putp(s) tputs((s), 1, fk_u_putchar)
#endif

extern int cur_on;

//#ifdef NCURSES
#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
#define fk_getch() getch()
#else
#ifdef HAVE_LIBGPM
#include <gpm.h>
#define fk_getch() Gpm_Getc(stdin)
#else
#define fk_getch() fgetc(stdin)
#endif
#endif

#define WpeExit(n) e_exit((n))

extern char *cur_rc, *cur_vs, *cur_nvs, *cur_vvs, *schirm;
extern char *att_no, *att_so, *att_ul, *att_rv, *att_bl, *att_dm, *att_bo;
extern int cur_x, cur_y;
extern char *user_shell;

extern char *ctree[5];

#ifdef NEWSTYLE
#define e_pr_char(x, y, c, frb)   \
(  *(schirm + 2*MAXSCOL*(y) + 2*(x)) = (c),  \
   *(schirm + 2*MAXSCOL*(y) + 2*(x) + 1) = (frb), \
   *(extbyte + MAXSCOL*(y) + (x)) = 0  )
#else
#define e_pr_char(x, y, c, frb)   \
(  *(schirm + 2*MAXSCOL*(y) + 2*(x)) = (c),  \
   *(schirm + 2*MAXSCOL*(y) + 2*(x) + 1) = (frb)  )
#endif
#define e_pt_col(x, y, c)  ( *(schirm + 2*MAXSCOL*(y) + 2*(x) + 1) = (c) )
#define e_gt_char(x, y)  (*(schirm + 2*MAXSCOL*(y) + 2*(x)))

#define e_gt_col(x, y)  (*(schirm + 2*MAXSCOL*(y) + 2*(x)+1))

#define e_gt_byte(x, y)  (*(schirm + 2*MAXSCOL*(y) + (x)))

#define e_pt_byte(x, y, c)  ( *(schirm + 2*MAXSCOL*(y) + (x)) = (c) )

/*  Pointer to functions for function calls  */

#define fk_locate(x, y) (*fk_u_locate)(x, y)
#define fk_cursor(x) (*fk_u_cursor)(x)
#define e_refresh() (*e_u_refresh)()
#define e_initscr(argc, argv) (*e_u_initscr)(argc, argv)
#define e_getch() (*e_u_getch)()
#define fk_putchar(c) (*fk_u_putchar)(c)
#define e_d_switch_out(c) (*e_u_d_switch_out)(c)
#define e_deb_out(window) (*e_u_deb_out)(window)
#define e_cp_X_to_buffer(window) (*e_u_cp_X_to_buffer)(window)
#define e_copy_X_buffer(window) (*e_u_copy_X_buffer)(window)
#define e_paste_X_buffer(window) (*e_u_paste_X_buffer)(window)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  bioskey - Get the status of shift, alt, and control keys.

    Returns: A bit field of the following info
      Bit  Information
       3   Alt key
       2   Control key
       1   Left shift
       0   Right shift
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define bioskey() (*u_bioskey)()
#define e_frb_menue(sw, xa, ya, window, md) (*e_frb_u_menue)(sw, xa, ya, window, md)
#define e_pr_col_kasten(xa, ya, x, y, window, sw) \
		(*e_pr_u_col_kasten)(xa, ya, x, y, window, sw)
#define e_s_clr(window, b) (*e_s_u_clr)(window, b)
#define e_n_clr(fb) (*e_n_u_clr)(fb)

#ifdef NEWSTYLE
extern char *extbyte, *altextbyte;
#endif
