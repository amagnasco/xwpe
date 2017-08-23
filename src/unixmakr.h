#ifndef UNIXMAKR_H
#define UNIXMAKR_H

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

extern int cur_on;
extern char *cur_rc, *cur_vs, *cur_nvs, *cur_vvs, *global_screen;
extern char *att_no, *att_so, *att_ul, *att_rv, *att_bl, *att_dm, *att_bo;
extern int cur_x, cur_y;
extern char *user_shell;
extern char *ctree[5];

#ifdef NEWSTYLE
extern char *extbyte, *altextbyte;
#endif

#endif
