#ifndef WE_UNIX_H
#define WE_UNIX_H

/** \file we_unix.h */

#include "config.h"
#include "globals.h"
#include "we_e_aus.h"
#include "we_edit.h"
#include "we_main.h"
#include "we_mouse.h"
#include "we_opt.h"

typedef enum wpeMouseShape
{
    WpeEditingShape,
    WpeDebuggingShape,
    WpeWorkingShape,
    WpeErrorShape,
    WpeSelectionShape,
    WpeLastShape
} WpeMouseShape;

/*   we_unix.c   */
int e_abs_refr(void);
void e_refresh_area(int x, int y, int width, int height);
void WpeNullFunction(void);
int WpeZeroFunction();
int e_tast_sim(int c);
void e_err_save(void);
void e_exit(int n);
char* e_mkfilepath(char* dr, char* fn, char* fl);
int e_compstr(char* a, char* b);
struct dirfile* e_find_files(char* sufile, int sw);
struct dirfile* e_find_dir(char* sufile, int sw);
char* e_file_info(char* filen, char* str, int* num, int sw);
void ini_repaint(we_control_t* control);
void end_repaint(void);
int e_frb_t_menue(int sw, int xa, int ya, we_window_t* window, int md);
void e_pr_t_col_kasten(int xa, int ya, int x, int y, we_window_t* window, int sw);
int e_ini_unix(int* argc, char** argv);
int e_recover(we_control_t* control);
int e_ini_schirm(int argc, char** argv);

extern int (*fk_u_locate)(int x, int y);
extern int (*fk_u_cursor)(int x);
extern int (*fk_u_putchar)(int c);
extern int (*u_bioskey)(void);
extern int (*e_frb_u_menue)(int sw, int xa, int ya, we_window_t* window, int md);
extern we_color_t (*e_s_u_clr)(int fg_color, int bg_color);
extern we_color_t (*e_n_u_clr)(int fg_bg_color);
extern void (*e_pr_u_col_kasten)(int xa, int ya, int x,
                                 int y, we_window_t* window, int sw);
extern int (*fk_mouse)(int g[]);
extern int (*e_u_refresh)(void);
extern int (*e_u_getch)(void);
extern int (*e_u_sys_ini)(void);
extern int (*e_u_sys_end)(void);
extern void (*WpeMouseChangeShape)(WpeMouseShape new_shape);
extern void (*WpeMouseRestoreShape)(void);
extern int (*e_u_d_switch_out)(int sw);
extern int (*e_u_switch_screen)(int sw);
extern int (*e_u_deb_out)(we_window_t* window);
extern int (*e_u_cp_X_to_buffer)(we_window_t* window);
extern int (*e_u_copy_X_buffer)(we_window_t* window);
extern int (*e_u_paste_X_buffer)(we_window_t* window);
extern int (*e_u_change)(we_view_t* view);
//extern int (*e_u_ini_size)(void);
extern int (*e_u_s_sys_end)(void);
extern int (*e_u_s_sys_ini)(void);
extern void (*e_u_setlastpic)(we_view_t* view);
extern int (*e_u_system)(const char* exe);
extern int (*e_u_kbhit)(void);
extern void (*WpeDisplayEnd)(void);

extern char e_we_sw;

#endif
