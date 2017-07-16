#ifndef WE_WIND_H
#define WE_WIND_H

#include "globals.h"
#include "we_main.h"
#include "we_mouse.h"

/*******************************************************************************/
/* (we_window *)f                                   */
/*            |                                   */
/*            +->(POINT)e.(int)x,y                */
/*            |         e.x=max_visible_cols_+1   */
/*            |         e.y=max_visible_lines_+1  */
/*            |                                   */
/*            +->(POINT)a.(int)x,y                */
/*            |         a.x=seems to always be 0  */
/*            |         a.y=seems to always be 1  */
/*            |                                   */
/*            +->(we_screen)s                        */
/*            |                                   */
/*            |  there's lots more in this struct */
/*            |                                   */
#define NUM_LINES_OFF_SCREEN_TOP	(f->s->c.y)
#define NUM_LINES_ON_SCREEN	(f->e.y - f->a.y)
#define LINE_NUM_ON_SCREEN_BOTTOM	(NUM_LINES_ON_SCREEN + NUM_LINES_OFF_SCREEN_TOP - 1)

/*this seems to include the scroll bar to the right*/
/*it's values is always +1 to the actual visible columns*/
#define NUM_COLS_ON_SCREEN_SAFE (((f->e.x - f->a.x) < (f->b->mx.x+1)) ? (f->e.x - f->a.x) : (f->b->mx.x+1))
#define NUM_COLS_ON_SCREEN	(f->e.x - f->a.x)

#define NUM_COLS_OFF_SCREEN_LEFT	(f->s->c.x)
#define COL_NUM_ON_SCREEN_RIGHT	(NUM_COLS_ON_SCREEN + NUM_COLS_OFF_SCREEN_LEFT - 1)

/*(LINE_NUM_ON_SCREEN_BOTTOM,COL_NUM_ON_SCREEN_RIGHT)*/
/*	is the coordinate for the lower right corner*/
/******************************************************************************/


/*   we_wind.c   */
int e_error (char *text, int sw, we_colorset * f);
int e_message (int sw, char *str, we_window * f);
void e_firstl (we_window * f, int sw);
int e_pr_filetype (we_window * f);
view *e_open_view (int xa, int ya, int xe, int ye, int col, int sw);
int e_close_view (view * pic, int sw);
void e_pr_line (int y, we_window * f);
void e_std_rahmen (int xa, int ya, int xe, int ye, char *name, int sw,
		   int frb, int fes);
void e_ed_rahmen (we_window * f, int sw);
int e_schirm (we_window * f, int sw);
int e_size_move (we_window * f);
view *e_std_kst (int xa, int ya, int xe, int ye, char *name, int sw, int fr,
		 int ft, int fes);
view *e_ed_kst (we_window * f, view * pic, int sw);
int e_close_window (we_window * f);
void e_switch_window (int num, we_window * f);
int e_ed_zoom (we_window * f);
int e_ed_cascade (we_window * f);
int e_ed_tile (we_window * f);
int e_ed_next (we_window * f);
int e_mess_win (char *header, char *str, view ** pic, we_window * f);
view *e_change_pic (int xa, int ya, int xe, int ye, view * pic, int sw,
		    int frb);
struct dirfile *e_add_df (char *str, struct dirfile *df);
int e_schr_nchar_wsv (char *str, int x, int y, int n, int max, int col,
		      int csw);
int e_schr_lst_wsv (char *str, int xa, int ya, int n, int strlen, int ft,
		    int fz, struct dirfile **df, we_window * f);
int e_rep_win_tree (ECNT * cn);
int e_opt_sec_box (int xa, int ya, int num, OPTK * opt, we_window * f, int sw);
int e_close_buffer (BUFFER * b);
int e_list_all_win (we_window * f);

#endif
