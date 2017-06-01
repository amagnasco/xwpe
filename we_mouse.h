#ifndef WE_MOUSE_H
#define WE_MOUSE_H

#include "we_wind.h"

/*   we_mouse.c   */
#if  MOUSE
int e_mshit(void);
int e_m1_mouse(void);
int e_m2_mouse(int xa, int ya, int xe, int ye, OPTK *fopt);
int e_m3_mouse(void);
int e_er_mouse(int x, int y, int xx, int yy);
int e_msg_mouse(int x, int y, int x1, int x2, int yy);
int WpeMngMouseInFileManager(FENSTER *f);
int WpeMouseInFileDirList(int k, int sw, FENSTER *f);
int fl_wnd_mouse(int sw, int k, FLWND *fw);
int e_lst_mouse(int x, int y, int n, int sw, int max, int nf);
void e_eck_mouse(FENSTER *f, int sw);
int e_edt_mouse(int c, FENSTER *f);
int e_ccp_mouse(int c, FENSTER *f);
void e_cur_mouse(FENSTER *f);
int e_opt_ck_mouse(int xa, int ya, int md);
int e_opt_cw_mouse(int xa, int ya, int md);
int e_opt_bs_mouse(void);
void e_opt_eck_mouse(W_OPTSTR *o);
int e_opt_mouse(W_OPTSTR *o);

int e_data_ein_mouse(FENSTER *f);
int e_opt_bs_mouse_1(void);
int e_opt_bs_mouse_2(void);
int e_opt_bs_mouse_3(void);
int e_rahmen_mouse(FENSTER *f);
#endif // #if MOUSE

#endif
