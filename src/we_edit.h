#ifndef WE_EDIT_H
#define WE_EDIT_H

#include "globals.h"
#include "we_block.h"
#include "we_wind.h"
#include "we_e_aus.h"
#include "we_fl_unix.h"
#include "we_fl_fkt.h"
#include "we_menue.h"
#include "we_mouse.h"
#include "we_debug.h"

/*   we_edit.c   */
int e_edit (ECNT * cn, char *filename);
int e_eingabe (ECNT * e);
int e_tst_cur (int c, ECNT * e);
int e_tst_fkt (int c, ECNT * e);
int e_ctrl_k (We_window * f);
int e_ctrl_o (We_window * f);
int e_tst_dfkt (We_window * f, int c);
int e_blk (int anz, int xa, int ya, int col);
int e_car_ret (BUFFER * b, we_screen * s);
void e_cursor (We_window * f, int sw);
int e_del_line (int yd, BUFFER * b, we_screen * s);
int e_del_nchar (BUFFER * b, we_screen * s, int x, int y, int n);
int e_ins_nchar (BUFFER * b, we_screen * sch, unsigned char *s, int xa, int ya,
		 int n);
int e_new_line (int yd, BUFFER * b);
int e_put_char (int c, BUFFER * b, we_screen * s);
int e_su_lblk (int xa, unsigned char *s);
int e_su_rblk (int xa, unsigned char *s);
void e_zlsplt (We_window * f);
void WpeFilenameToPathFile (char *filename, char **path, char **file);
int e_lst_zeichen (int x, int y, int n, int sw, int frb, int max, int iold,
		   int new);
void e_mouse_bar (int x, int y, int n, int sw, int frb);
int e_chr_sp (int x, BUFFER * b, We_window * f);
Undo *e_remove_undo (Undo * ud, int sw);
int e_add_undo (int sw, BUFFER * b, int x, int y, int n);
int e_make_undo (We_window * f);
int e_make_redo (We_window * f);
int e_make_rudo (We_window * f, int sw);
int e_autosave (We_window * f);
char *e_make_postf (char *out, char *name, char *pf);

#endif
