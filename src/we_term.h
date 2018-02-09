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

extern char *global_screen;
extern char *global_alt_screen;

#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
extern char *extbyte;
extern char *altextbyte;
#endif

extern char *att_no;
extern int col_num;
extern int cur_x;
extern int cur_y;
extern char *ctree[5];

int e_abs_refr(void);
int e_alloc_global_screen(void);
int max_screen_lines();
int set_max_screen_lines(const int max_lines);
int max_screen_cols();
int set_max_screen_cols(const int max_cols);

#endif
