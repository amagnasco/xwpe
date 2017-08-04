#ifndef WE_DEBUG_H
#define WE_DEBUG_H

#include "config.h"
#include "globals.h"
#include "we_wind.h"

#ifdef DEBUGGER
int e_deb_inp (We_window * f);
int e_e_line_read (int n, char *s, int max);
int e_d_dum_read (void);
int e_d_p_exec (We_window * f);
int e_d_getchar (void);
/** return changed to void: no return was given and no one tested return. */
void e_d_quit_basic (We_window * f);
int e_d_quit (We_window * f);
int e_d_add_watch (char *str, We_window * f);
int e_remove_all_watches (We_window * f);
int e_make_watches (We_window * f);
int e_edit_watches (We_window * f);
int e_delete_watches (We_window * f);
int e_d_p_watches (We_window * f, int sw);
int e_deb_stack (We_window * f);
int e_d_p_stack (We_window * f, int sw);
int e_make_stack (We_window * f);
int e_breakpoint (We_window * f);
int e_remove_breakpoints (We_window * f);
int e_make_breakpoint (We_window * f, int sw);
int e_exec_deb (We_window * f, char *prog);
int e_start_debug (We_window * f);
int e_run_debug (We_window * f);
int e_deb_run (We_window * f);
int e_deb_trace (We_window * f);
int e_deb_next (We_window * f);
int e_d_goto_cursor (We_window * f);
int e_d_finish_func (We_window * f);
int e_deb_options (We_window * f);
int e_d_step_next (We_window * f, int sw);
int e_read_output (We_window * f);
int e_d_pr_sig (char *str, We_window * f);
int e_make_line_num (char *str, char *file);
int e_make_line_num2 (char *str, char *file);
int e_d_goto_break (char *file, int line, We_window * f);
int e_d_is_watch (int c, We_window * f);
int e_debug_switch (We_window * f, int c);
int e_d_putchar (int c);
int e_g_sys_ini (void);
int e_g_sys_end (void);
int e_test_command (char *str);

/**** functions for breakpoints resyncing, reloading etc ****/
int e_brk_schirm (We_window * f);
int e_brk_recalc (We_window * f, int start, int len);
int e_d_reinit_brks (We_window * f, char *prj);

/**** for reloading watches ****/
int e_d_reinit_watches (We_window * f, char *prj);

#endif // #ifdef DEBUGGER

#endif
