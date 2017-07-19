#ifndef WE_BLOCK_H
#define WE_BLOCK_H

#include "we_fl_unix.h"
#include "we_opt.h"
#include "we_debug.h"

int e_blck_del (We_window * f);
int e_blck_dup (char *dup, We_window * f);
int e_show_clipboard (We_window * f);
int e_edt_del (We_window * f);
int e_edt_copy (We_window * f);
int e_edt_einf (We_window * f);
int e_blck_move (We_window * f);
void e_move_block (int x, int y, BUFFER * bv, BUFFER * bz, We_window * f);
int e_blck_copy (We_window * f);
void e_copy_block (int x, int y, BUFFER * buffer_src, BUFFER * buffer_dst,
		   We_window * f);
int e_blck_begin (We_window * f);
int e_blck_end (We_window * f);
int e_blck_hide (We_window * f);
int e_find (We_window * f);
int e_replace (We_window * f);
int e_goto_line (We_window * f);
int e_changecase_dialog (We_window * f);
int e_blck_to_left (We_window * f);
int e_blck_to_right (We_window * f);
int e_blck_read (We_window * f);
int e_blck_write (We_window * f);
int e_rep_search (We_window * f);

#endif
