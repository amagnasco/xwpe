#ifndef WE_TERM_H
#define WE_TERM_H

/** \file we_term.h */

#include "config.h"

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

/**
 * tputs is necessary to add padding for termcap or terminfo
 * capabilities. For curses or ncurses this function is not
 * necessary
 */
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
#define e_putp(s) tputs((s), 1, fk_u_putchar)
#endif

extern int cur_x;
extern int cur_y;
extern char *ctree[5];

int e_abs_refr(void);
int e_alloc_global_screen(void);

#endif
