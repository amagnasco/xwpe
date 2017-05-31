#ifndef WE_BLOCK_H
#define WE_BLOCK_H

#include "we_edit.h"
#include "we_fl_unix.h"

int e_blck_del(FENSTER *f);
int e_blck_dup(char *dup, FENSTER *f);
int e_show_clipboard(FENSTER *f);
int e_edt_del(FENSTER *f);
int e_edt_copy(FENSTER *f);
int e_edt_einf(FENSTER *f);
int e_blck_move(FENSTER *f);
void e_move_block(int x, int y, BUFFER *bv, BUFFER *bz, FENSTER *f);
int e_blck_copy(FENSTER *f);
void e_copy_block(int x, int y, BUFFER *buffer_src, BUFFER *buffer_dst,
  FENSTER *f);
int e_blck_begin(FENSTER *f);
int e_blck_end(FENSTER *f);
int e_blck_hide(FENSTER *f);
int e_find(FENSTER *f);
int e_replace(FENSTER *f);
int e_goto_line(FENSTER *f);
int e_changecase_dialog(FENSTER *f);
int e_blck_to_left(FENSTER *f);
int e_blck_to_right(FENSTER *f);
int e_blck_read(FENSTER *f);
int e_blck_write(FENSTER *f);
int e_rep_search(FENSTER *f);

#endif
