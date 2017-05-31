#ifndef WE_EDIT_H
#define WE_EDIT_H

#include "globals.h"
#include "we_block.h"
#include "we_wind.h"
#include "we_e_aus.h"

/*   we_edit.c   */
int e_edit(ECNT *cn, char *filename);
int e_eingabe(ECNT *e);
int e_tst_cur(int c, ECNT *e);
int e_tst_fkt(int c, ECNT *e);
int e_ctrl_k(FENSTER *f);
int e_ctrl_o(FENSTER *f);
int e_tst_dfkt(FENSTER *f, int c);
int e_blk(int anz, int xa, int ya, int col);
int e_car_ret(BUFFER *b, SCHIRM *s);
void e_cursor(FENSTER *f, int sw);
int e_del_line(int yd, BUFFER *b, SCHIRM *s);
int e_del_nchar(BUFFER *b, SCHIRM *s, int x, int y, int n);
int e_ins_nchar(BUFFER *b, SCHIRM *sch, unsigned char *s, int xa, int ya,
  int n);
int e_new_line(int yd, BUFFER *b);
int e_put_char(int c, BUFFER *b, SCHIRM *s);
int e_su_lblk(int xa, char *s);
int e_su_rblk(int xa, char *s);
void e_zlsplt(FENSTER *f);
void WpeFilenameToPathFile(char *filename, char **path, char **file);
int e_lst_zeichen(int x, int y, int n, int sw, int frb, int max, int iold,
  int new);
void e_mouse_bar(int x, int y, int n, int sw, int frb);
int e_chr_sp(int x, BUFFER *b, FENSTER *f);
Undo *e_remove_undo(Undo *ud, int sw);
int e_add_undo(int sw, BUFFER *b, int x, int y, int n);
int e_make_undo(FENSTER *f);
int e_make_redo(FENSTER *f);
int e_make_rudo(FENSTER *f, int sw);
int e_autosave(FENSTER *f);
char *e_make_postf(char *out, char *name, char *pf);

#endif
