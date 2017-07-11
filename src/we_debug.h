#ifndef WE_DEBUG_H
#define WE_DEBUG_H

#include "globals.h"
#include "we_wind.h"

#ifdef DEBUGGER
int e_deb_inp(FENSTER *f);
int e_e_line_read(int n, char *s, int max);
int e_d_dum_read(void);
int e_d_p_exec(FENSTER *f);
int e_d_getchar(void);
int e_d_quit_basic(FENSTER *f);
int e_d_quit(FENSTER *f);
int e_d_add_watch(char *str, FENSTER *f);
int e_remove_all_watches(FENSTER *f);
int e_make_watches(FENSTER *f);
int e_edit_watches(FENSTER *f);
int e_delete_watches(FENSTER *f);
int e_d_p_watches(FENSTER *f, int sw);
int e_deb_stack(FENSTER *f);
int e_d_p_stack(FENSTER *f, int sw);
int e_make_stack(FENSTER *f);
int e_breakpoint(FENSTER *f);
int e_remove_breakpoints(FENSTER *f);
int e_make_breakpoint(FENSTER *f, int sw);
int e_exec_deb(FENSTER *f, char *prog);
int e_start_debug(FENSTER *f);
int e_run_debug(FENSTER *f);
int e_deb_run(FENSTER *f);
int e_deb_trace(FENSTER *f);
int e_deb_next(FENSTER *f);
int e_d_goto_cursor(FENSTER *f);
int e_d_finish_func(FENSTER *f);
int e_deb_options(FENSTER *f);
int e_d_step_next(FENSTER *f, int sw);
int e_read_output(FENSTER *f);
int e_d_pr_sig(char *str, FENSTER *f);
int e_make_line_num(char *str, char *file);
int e_make_line_num2(char *str, char *file);
int e_d_goto_break(char *file, int line, FENSTER *f);
int e_d_is_watch(int c, FENSTER *f);
int e_debug_switch(FENSTER *f, int c);
int e_d_putchar(int c);
int e_g_sys_ini(void);
int e_g_sys_end(void);
int e_test_command(char *str);

/**** functions for breakpoints resyncing, reloading etc ****/
int e_brk_schirm(FENSTER *f);
int e_brk_recalc(FENSTER *f,int start,int len);
int e_d_reinit_brks(FENSTER *f,char * prj);

/**** for reloading watches ****/
int e_d_reinit_watches(FENSTER *f,char * prj);

#endif // #ifdef DEBUGGER

#endif
