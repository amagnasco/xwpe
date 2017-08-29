#ifndef WE_DEBUG_H
#define WE_DEBUG_H
/** \file we_debug.h */

#include "config.h"
#include "globals.h"
#include "we_wind.h"

#ifdef DEBUGGER
int e_deb_inp(we_window_t* window);
int e_e_line_read(int n, char* s, int max);
int e_d_dum_read(void);
int e_d_p_exec(we_window_t* window);
int e_d_getchar(void);
/** return changed to void: no return was given and no one tested return. */
void e_d_quit_basic(we_window_t* window);
int e_d_quit(we_window_t* window);
int e_d_add_watch(char* str, we_window_t* window);
int e_remove_all_watches(we_window_t* window);
int e_make_watches(we_window_t* window);
int e_edit_watches(we_window_t* window);
int e_delete_watches(we_window_t* window);
int e_d_p_watches(we_window_t* window, int sw);
int e_deb_stack(we_window_t* window);
int e_d_p_stack(we_window_t* window, int sw);
int e_make_stack(we_window_t* window);
int e_breakpoint(we_window_t* window);
int e_remove_breakpoints(we_window_t* window);
int e_make_breakpoint(we_window_t* window, int sw);
int e_exec_deb(we_window_t* window, char* prog);
int e_start_debug(we_window_t* window);
int e_run_debug(we_window_t* window);
int e_deb_run(we_window_t* window);
int e_deb_trace(we_window_t* window);
int e_deb_next(we_window_t* window);
int e_d_goto_cursor(we_window_t* window);
int e_d_finish_func(we_window_t* window);
int e_deb_options(we_window_t* window);
int e_d_step_next(we_window_t* window, int sw);
int e_read_output(we_window_t* window);
int e_d_pr_sig(char* str, we_window_t* window);
int e_make_line_num(char* str, char* file);
int e_make_line_num2(char* str, char* file);
int e_d_goto_break(char* file, int line, we_window_t* window);
int e_d_is_watch(int c, we_window_t* window);
int e_debug_switch(we_window_t* window, int c);
int e_d_putchar(int c);
int e_g_sys_ini(void);
int e_g_sys_end(void);
int e_test_command(char* str);

/**** functions for breakpoints resyncing, reloading etc ****/
int e_brk_schirm(we_window_t* window);
int e_brk_recalc(we_window_t* window, int start, int len);
int e_d_reinit_brks(we_window_t* window, char* prj);

/**** for reloading watches ****/
int e_d_reinit_watches(we_window_t* window, char* prj);

#endif // #ifdef DEBUGGER

#endif
