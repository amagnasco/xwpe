#ifndef WE_OPT_H
#define WE_OPT_H

#include "globals.h"
#include "we_main.h"
#include "we_mouse.h"
#include "we_xterm.h"

/*   we_opt.c   */
char *WpeStringToValue (const char *str);
char *WpeValueToString (const char *value);
int e_about_WE (we_window * f);
int e_clear_desk (we_window * f);
int e_repaint_desk (we_window * f);
int e_sys_info (we_window * f);
int e_ad_colors (we_window * f);
int e_dif_colors (int sw, int xa, int ya, we_window * f, int md);
void e_pr_dif_colors (int sw, int xa, int ya, we_window * f, int sw2, int md);
void e_pr_x_col_kasten (int xa, int ya, int x, int y, we_window * f, int sw);
void e_pr_ed_beispiel (int xa, int ya, we_window * f, int sw, int md);
int e_opt_save (we_window * f);
int e_save_opt (we_window * f);
int e_opt_read (ECNT * cn);
int e_add_arguments (char *str, char *head, we_window * f, int n, int sw,
		     struct dirfile **df);
W_O_TXTSTR **e_add_txtstr (int x, int y, char *txt, W_OPTSTR * o);
W_O_WRSTR **e_add_wrstr (int xt, int yt, int xw, int yw, int nw, int wmx,
			 int nc, int sw, char *header, char *txt,
			 struct dirfile **df, W_OPTSTR * o);
W_O_NUMSTR **e_add_numstr (int xt, int yt, int xw, int yw, int nw, int wmx,
			   int nc, int sw, char *header, int num,
			   W_OPTSTR * o);
W_O_SSWSTR **e_add_sswstr (int x, int y, int nc, int sw, int num,
			   char *header, W_OPTSTR * o);
W_O_SPSWSTR **e_add_spswstr (int n, int x, int y, int nc, int sw,
			     char *header, W_OPTSTR * o);
W_O_PSWSTR **e_add_pswstr (int n, int x, int y, int nc, int sw, int num,
			   char *header, W_OPTSTR * o);
W_O_BTTSTR **e_add_bttstr (int x, int y, int nc, int sw, char *header,
			   int (*fkt) (we_window * f), W_OPTSTR * o);
int freeostr (W_OPTSTR * o);
W_OPTSTR *e_init_opt_kst (we_window * f);
int e_opt_move (W_OPTSTR * o);
int e_get_sw_cmp (int xin, int yin, int x, int y, int xmin, int ymin, int c);
int e_get_opt_sw (int c, int x, int y, W_OPTSTR * o);
int e_opt_kst (W_OPTSTR * o);
int e_edt_options (we_window * f);
int e_read_colors (we_window * f);
int e_ad_colors_md (we_window * f, int md);
int e_frb_x_menue (int sw, int xa, int ya, we_window * f, int md);
void e_pr_x_col_kasten (int xa, int ya, int x, int y, we_window * f, int sw);

#endif
